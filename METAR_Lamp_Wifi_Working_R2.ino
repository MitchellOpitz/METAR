// 1: Organize includes
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
#include <EEPROM.h>
#include <WiFiManager.h>

using namespace std;

// 2: Define constants
#define NUM_AIRPORTS 25
#define LOOP_INTERVAL 300000
#define TRIGGER_PIN D4
#define DATA_PIN    D2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define SERVER "aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString="
#define POT_PIN A0
#define REPROGRAM_BUTTON_PIN D3

// Step 3: Initialize variables
int timeout = 120;
String airports = "";
String airportString = "";
WiFiManager wifiManager;
CRGB leds[NUM_AIRPORTS];
String data;
unsigned int loops = -1;
int status = WL_IDLE_STATUS;

// Possibly unused
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define USE_LIGHT_SENSOR false
#define LIGHT_SENSOR_TSL2561 false
#define BRIGHT_PIN A0

/* ----------------------------------------------------------------------- */

 
#define DEBUG false  // MO - Add debug mode
boolean ledStatus = true; // used so leds only indicate connection status on first boot, or after failure


void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);

    setupWifi();    
    initializePins();
    initializeLeds();
    indicateWifiStatus();
}

void loop() {
    if (isReprogramButtonPressed()) {
        enterReprogramMode();
    } else {
        connectToWifi();
        readAirportData();
        String metarData = retrieveMetarData(airports);
        parseMetarData(metarData);
        doColor();
        updateBrightness();
        delay(LOOP_INTERVAL);
    }
}

void setupWiFi() {
  // Configure WiFi mode
  WiFi.mode(WIFI_STA);

  // Configure WiFi manager
  wifiManager.setDebugOutput(false);
  wifiManager.resetSettings();

  // Setup WiFi connection parameters
  WiFiManagerParameter custom_text_box("ICAO", "Enter Your Airport Here", "", 4);
  wifiManager.addParameter(&custom_text_box);

  // Save custom parameter if entered
  String data = custom_text_box.getValue();
  if (data != "") {
    writeStringToEEPROM(10, data);
  }
}

void doColor(String identifier, unsigned short int led, int wind, int gusts, String condition, String wxstring) {
    CRGB color;

    if (condition == "LIFR" || identifier == "LIFR") {
        color = CRGB::Magenta;
    } else if (condition == "IFR") {
        color = CRGB::Red;
    } else if (condition == "MVFR") {
        color = CRGB::Blue;
    } else if (condition == "VFR") {
        color = CRGB::Green;
    } else {
        color = CRGB::Black;
    }

    Serial.print(identifier);
    Serial.print(": ");
    Serial.print(condition);
    Serial.print(" ");
    Serial.print(wind);
    Serial.print("G");
    Serial.print(gusts);
    Serial.print("kts LED ");
    Serial.print(led);
    Serial.print(" WX: ");
    Serial.println(wxstring);

    leds[led] = color;
}

void initializeLeds() {
    int BRIGHTNESS = analogRead(BRIGHT_PIN);
    BRIGHTNESS = BRIGHTNESS / 4.5; 
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

void initializePins() {
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(REPROGRAM_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

// Section - Wifi Handling

void connectToWifi() {
    wifiManager.setConfigPortalBlocking(true);  // Disables WifI configuration portal.  May not be needed.
    if (!wifiManager.autoConnect("MetarWiFi")) {
        Serial.println("Failed to connect to WiFi or hit timeout.");
    } else {
        Serial.println("Connected to WiFi.");
    }
}

void indicateWifiStatus() {
    fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
    FastLED.show();
}

// Section - EEPROM

void writeStringToEEPROM(char add, String data) {
  int _size = data.length();
  for (int i = 0; i < _size; i++) {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0');
  EEPROM.commit();
}

String readStringFromEEPROM(char add) {
  const int maxLength = 100;
  char data[maxLength + 1];
  int len = 0;
  char currentChar;

  do {
    currentChar = EEPROM.read(add + len);
    data[len] = currentChar;
    len++;
  } while (currentChar != '\0' && len < maxLength);

  data[len] = '\0';

  return String(data);
}

// Section - Data Handling

 void readAirportData() {
    airports = readStringFromEEPROM(10);
    Serial.print("Airport: ");
    Serial.println(airports);
}

String retrieveMetarData(String airports) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        client.stop();
        return ""; // Return an empty string indicating failure
    }

    // Construct and send HTTP request
    String request = "GET " + String(BASE_URI) + airports + " HTTP/1.1\r\n" +
                     "Host: " + String(SERVER) + "\r\n" +
                     "User-Agent: LED Sectional Client\r\n" +
                     "Connection: close\r\n\r\n";
    client.print(request);

    // Wait for response
    unsigned long timeout = millis();
    while (!client.available()) {
        if (millis() - timeout > 5000) {
            Serial.println("Timeout waiting for response");
            client.stop();
            return ""; // Return an empty string indicating timeout
        }
    }

    // Read and process response
    String responseData = "";
    while (client.available()) {
        // Read response data
        String line = client.readStringUntil('\r');
        responseData += line; // Append each line to the response data
    }

    client.stop();
    return responseData; // Return the received data
}

void parseMetarData(String data) {
  // Define starting tags for each relevant field
  const String TAG_STATION_ID = "<station_id>";
  const String TAG_WIND_SPEED = "<wind_speed_kt>";
  const String TAG_WIND_GUST = "<wind_gust_kt>";
  const String TAG_FLIGHT_CATEGORY = "<flight_category>";
  const String TAG_WX_STRING = "<wx_string>";

  // Initialize variables
  String currentAirport;
  int currentWind = 0;
  int currentGusts = 0;
  String currentCondition;
  String currentWxstring;

  // Iterate through each line in the data
  for (int i = 0; i < data.length(); i++) {
    if (data.charAt(i) == '\n') { // Check for line break
      // Process the collected data for the current airport
      processLine(currentAirport, currentCondition, currentWind, currentGusts, currentWxstring);

      // Reset variables for the next airport
      currentAirport = "";
      currentWind = 0;
      currentGusts = 0;
      currentCondition = "";
      currentWxstring = "";
      continue;
    }

    // Check for starting tags and extract corresponding values
    int tagEnd = data.indexOf('>', i);
    if (tagEnd > -1) {
      String tag = data.substring(i, tagEnd + 1);
      i = tagEnd + 1; // Move index to after closing tag

      if (tag == TAG_STATION_ID) {
        currentAirport = data.substring(i, data.indexOf('<', i));
      } else if (tag == TAG_WIND_SPEED) {
        currentWind = data.substring(i, data.indexOf('<', i)).toInt();
      } else if (tag == TAG_WIND_GUST) {
        currentGusts = data.substring(i, data.indexOf('<', i)).toInt();
      } else if (tag == TAG_FLIGHT_CATEGORY) {
        currentCondition = data.substring(i, data.indexOf('<', i));
      } else if (tag == TAG_WX_STRING) {
        currentWxstring = data.substring(i, data.indexOf('<', i));
      }
    }
  }

  // Process the last airport data
  processLine(currentAirport, currentCondition, currentWind, currentGusts, currentWxstring);
}

void processLine(String currentAirport, String currentCondition, int currentWind, int currentGusts, String currentWxstring) {
    if (!currentAirport.isEmpty()) {
        for (unsigned short int i = 0; i < NUM_AIRPORTS; i++) {
            if (airports == currentAirport) {
                doColor(currentAirport, i, currentWind, currentGusts, currentCondition, currentWxstring);
            }
        }
    }
}

void updateBrightness() {
    int potValue = analogRead(POT_PIN);
    int brightness = map(potValue, 0, 1023, 0, 255);
    FastLED.setBrightness(brightness);
}

bool isReprogramButtonPressed() {
  return digitalRead(REPROGRAM_BUTTON_PIN) == LOW;
}

void enterReprogramMode() {
    Serial.println("Reprogram mode activated.");
}
