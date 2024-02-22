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
#define DATA_PIN D2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define SERVER "aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString="
#define POT_PIN A0
#define REPROGRAM_BUTTON_PIN D3

// Step 3: Initialize variables
int timeout = 120;
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_AIRPORTS];
String data;

// Possibly unused
#define BRIGHT_PIN A0

/* ----------------------------------------------------------------------- */

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);

    setupWiFi();
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

void initializeLeds() {    
    int BRIGHTNESS = analogRead(BRIGHT_PIN);
    BRIGHTNESS = BRIGHTNESS / 4.5;
    BRIGHTNESS = 5;
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
    //wifiManager.setConfigPortalBlocking(true);  // Disables WifI configuration portal.  May not be needed.
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
    // Establish a secure HTTPS connection to the server
    BearSSL::WiFiClientSecure client;
    client.setInsecure(); // For development/testing only; use proper certificates in production

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        return ""; // Indicate failure
    }

    // Construct the HTTP request header with appropriate formatting
    String request = "GET " + String(BASE_URI) + airports + " HTTP/1.1\r\n";
                    "Host: " + String(SERVER) + "\r\n"
                    "User-Agent: LED Sectional Client\r\n"
                    "Connection: close\r\n\r\n";

    // Send the HTTP request
    client.print(request);

    // Set a reasonable timeout to prevent indefinite waiting
    unsigned long timeout = millis();
    const unsigned long maxTimeout = 5000; // 5 seconds

    // Receive and process the response
    String responseData = "";
    while (client.available()) {
        if (millis() - timeout > maxTimeout) {
            Serial.println("Timeout waiting for response");
            break; // Exit the loop on timeout
        }

        responseData += client.readStringUntil('\r'); // Read and append response lines
    }

    client.stop();

    return responseData; // Return the received data, even if empty due to errors
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
                setColor(currentCondition, i);
            }
        }
    }
}

void setColor(String condition, unsigned short int led) {
  CRGB color;

  if (condition == "LIFR") {
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

  leds[led] = color;
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
