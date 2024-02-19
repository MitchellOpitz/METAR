// 1: Organize includes
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
#include <EEPROM.h>
#include <WiFiManager.h>

using namespace std;

// 2: Define constants
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define NUM_AIRPORTS 25
#define LOOP_INTERVAL 300000
#define TRIGGER_PIN D4
#define USE_LIGHT_SENSOR false
#define LIGHT_SENSOR_TSL2561 false
#define DATA_PIN    D2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define BRIGHT_PIN A0
#define READ_TIMEOUT 15 // Cancel query if no data received (seconds)
#define WIFI_TIMEOUT 60 // in seconds
#define RETRY_TIMEOUT 15000 // in ms
#define SERVER "aviationweather.gov"
#define BASE_URI "/cgi-bin/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString="

// Step 3: Initialize variables
int timeout = 120;
String airports = "";
String airportString = "";
WiFiManager wifiManager;
CRGB leds[NUM_AIRPORTS];
String data;
unsigned int loops = -1;
int status = WL_IDLE_STATUS;

/* ----------------------------------------------------------------------- */

 
#define DEBUG false  // MO - Add debug mode
boolean ledStatus = true; // used so leds only indicate connection status on first boot, or after failure


void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);

    configureWiFi();
    configureWiFiManager();
    setupWiFiManager();
    setupCustomParameter();
    initializePins();
    initializeLeds();
    indicateWifiStatus();
}

void loop() {
    connectToWifi();
    readAirportData();
    retrieveMetarData();
    parseMetarData();
    doColor();
    handleDelay();
}

 void readAirportData() {
    airports = readStringFromEEPROM(10);
    Serial.print("Airport: ");
    Serial.println(airports);
}

bool retrieveMetarData(){
  fill_solid(leds, NUM_AIRPORTS, CRGB::Black); // Set everything to black just in case there is no report
  uint32_t t;
  char c;
  boolean readingAirport = false;
  boolean readingCondition = false;
  boolean readingWind = false;
  boolean readingGusts = false;
  boolean readingWxstring = false;

  std::vector<unsigned short int> led;
  String currentAirport = "";
  String currentCondition = "";
  String currentLine = "";
  String currentWind = "";
  String currentGusts = "";
  String currentWxstring = "";
  String airportString = "";
  bool firstAirport = true;
  for (int i = 0; i < NUM_AIRPORTS; i++) {
    if (airports != "NULL" && airports != "VFR" && airports != "MVFR" && airports != "WVFR" && airports != "IFR" && airports != "LIFR") {
      if (firstAirport) {
        firstAirport = false;
        airportString = airports;
      } else airportString = airportString + "," + airports;
    }
  }
  
  retrieveMetarData(airports);

    Serial.println();

parseMetarData();  // Need data string.

  // need to doColor this for the last airport
  for (vector<unsigned short int>::iterator it = led.begin(); it != led.end(); ++it) {
    doColor(currentAirport, *it, currentWind.toInt(), currentGusts.toInt(), currentCondition, currentWxstring);
  }
  led.clear();

  // Do the key LEDs now if they exist
  for (int i = 0; i < (NUM_AIRPORTS); i++) {
    // Use this opportunity to set colors for LEDs in our key then build the request string
    if (airports == "VFR") leds[i] = CRGB::Green;
    else if (airports == "MVFR") leds[i] = CRGB::Blue;
    else if (airports == "IFR") leds[i] = CRGB::Red;
    else if (airports == "LIFR") leds[i] = CRGB::Magenta;
  }

  client.stop();
  return true;
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

void connectToWifi() {
    wifiManager.setConfigPortalBlocking(true);  // Disables WifI configuration portal.  May not be needed.
    if (!wifiManager.autoConnect("MetarWiFi")) {
        Serial.println("Failed to connect to WiFi or hit timeout.");
    } else {
        Serial.println("Connected to WiFi.");
    }
}

void configureWiFiManager() {
    wifiManager.setDebugOutput(false);
    wifiManager.resetSettings();
}

void initializeLeds() {
    int BRIGHTNESS = analogRead(BRIGHT_PIN);
    BRIGHTNESS = BRIGHTNESS / 4.5; 
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

bool retrieveMetarData(String airports) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        client.stop();
        return false;
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
            return false;
        }
    }

    // Read and process response
    while (client.available()) {
        // Read response data
        String line = client.readStringUntil('\r');
        // Process response data (parse METARs)
    }

    client.stop();
    return true;
}

void parseMetarData(String data) {
    String currentAirport = "";
    String currentCondition = "";
    String currentWind = "";
    String currentGusts = "";
    String currentWxstring = "";

    bool readingAirport = false;
    bool readingWind = false;
    bool readingGusts = false;
    bool readingCondition = false;
    bool readingWxstring = false;

    for (int i = 0; i < data.length(); i++) {
        char c = data.charAt(i);

        if (c == '\n') {
            currentLine = "";
            continue;
        }

        currentLine += c;

        if (currentLine.endsWith("<station_id>")) {
            if (!led.empty()) {
                for (auto it = led.begin(); it != led.end(); ++it) {
                    doColor(currentAirport, *it, currentWind.toInt(), currentGusts.toInt(), currentCondition, currentWxstring);
                }
                led.clear();
            }
            currentAirport = "";
            readingAirport = true;
            currentCondition = "";
            currentWind = "";
            currentGusts = "";
            currentWxstring = "";
        } else if (readingAirport) {
            if (!currentLine.endsWith("<")) {
                currentAirport += c;
            } else {
                readingAirport = false;
                for (unsigned short int i = 0; i < NUM_AIRPORTS; i++) {
                    if (airports == currentAirport) {
                        led.push_back(i);
                    }
                }
            }
        } else if (currentLine.endsWith("<wind_speed_kt>")) {
            readingWind = true;
        } else if (readingWind) {
            if (!currentLine.endsWith("<")) {
                currentWind += c;
            } else {
                readingWind = false;
            }
        } else if (currentLine.endsWith("<wind_gust_kt>")) {
            readingGusts = true;
        } else if (readingGusts) {
            if (!currentLine.endsWith("<")) {
                currentGusts += c;
            } else {
                readingGusts = false;
            }
        } else if (currentLine.endsWith("<flight_category>")) {
            readingCondition = true;
        } else if (readingCondition) {
            if (!currentLine.endsWith("<")) {
                currentCondition += c;
            } else {
                readingCondition = false;
            }
        } else if (currentLine.endsWith("<wx_string>")) {
            readingWxstring = true;
        } else if (readingWxstring) {
            if (!currentLine.endsWith("<")) {
                currentWxstring += c;
            } else {
                readingWxstring = false;
            }
        }
    }
}

void configureWiFi() {
    WiFi.mode(WIFI_STA);
}

void setupWiFiManager() {
    WiFiManagerParameter custom_text_box("ICAO", "Enter Your Airport Here", "", 4);
    wifiManager.addParameter(&custom_text_box);
}

void setupCustomParameter() {
    String data = custom_text_box.getValue();
    if (data != "") {
        writeStringToEEPROM(10, data);
    }
}

void initializePins() {
    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void indicateWifiStatus() {
    fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
    FastLED.show();
}
