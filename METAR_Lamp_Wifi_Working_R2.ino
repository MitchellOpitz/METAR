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
#define BASE_URI "/api/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString="
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

    initializePins();
    initializeLeds();
    configureWifi();
}

void loop() {
    if (isReprogramButtonPressed()) {
        enterReprogramMode();
    } else {
        connectToAccessPoint();
        readAirportData();
        String metarData = retrieveMetarData(airports);
        if(!metarData.isEmpty()) {
            parseMetarData(metarData);
            updateBrightness();
        }
        delay(LOOP_INTERVAL);
    }
}

void initializePins() {    
    Serial.println("Initializing pins...");
    pinMode(TRIGGER_PIN, INPUT_PULLUP);    
    pinMode(REPROGRAM_BUTTON_PIN, INPUT_PULLUP);    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void initializeLeds() {    
    Serial.println("Initializing LEDs...");
    int BRIGHTNESS = analogRead(BRIGHT_PIN);
    BRIGHTNESS = BRIGHTNESS / 4.5;
    BRIGHTNESS = 5;
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_AIRPORTS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
    FastLED.show();
}

void configureWifi() {  
  Serial.println("Configuring wifi...");
  
  // Configure WiFi mode
  WiFi.mode(WIFI_STA);

  // Configure WiFi manager
  wifiManager.setDebugOutput(false);
  wifiManager.resetSettings();

  // Setup WiFi connection parameters
  WiFiManagerParameter custom_text_box("ICAO", "Enter Your Airport Here", "", 4);
  wifiManager.addParameter(&custom_text_box);

  // Connect to wifi config portal
  if (wifiManager.autoConnect("MetarWiFi")) {
        Serial.println("Successfully configured WiFi .");
    } else {
        Serial.println("Failed to configure WiFi or hit timeout.");
    }
    
    // Save custom parameter if entered
  String data = custom_text_box.getValue();
  if (data != "") {
    writeStringToEEPROM(10, data);
  }
}

// Section - Wifi Handling

void connectToAccessPoint() {
  Serial.println("Checking connection to access point...");
    if (wifiManager.autoConnect("MetarAP")) {
        Serial.println("Connected to WiFi.");
        fill_solid(leds, NUM_AIRPORTS, CRGB::Purple);
        FastLED.show();
    } else {
        Serial.println("Failed to connect to WiFi or hit timeout.");
        fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
        FastLED.show();
    }
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
  Serial.println("Reading airport data...");
    airports = readStringFromEEPROM(10);
    Serial.println("Airport: " + airports);
}

String retrieveMetarData(String airports) {
    Serial.println("Retrieving Metar data...");
    
    // Establish a secure HTTPS connection to the server
    BearSSL::WiFiClientSecure client;
    client.setInsecure(); // For development/testing only; use proper certificates in production

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        return ""; // Indicate failure
    }

    // Construct the HTTP request header with appropriate formatting
    String request = "GET " + String(BASE_URI) + airports + " HTTP/1.1\r\n" +
                    "Host: " + String(SERVER) + "\r\n" +
                    "User-Agent: LED Sectional Client\r\n" +
                    "Connection: close\r\n\r\n";

    // Send the HTTP request
    Serial.println("Sending request...");
    Serial.println(request);
    client.print(request);

    // Set a reasonable timeout to prevent indefinite waiting
    unsigned long timeout = millis();
    const unsigned long maxTimeout = 5000; // 5 seconds

    // Receive and process the response
    String responseData = "";
    while (client.connected() && !client.available()) {
        if (millis() - timeout > maxTimeout) {
            Serial.println("Timeout waiting for response");
            client.stop();
            return ""; // Indicate timeout
        }
    }

    // Read response data
    while (client.available()) {
        char c = client.read();
        responseData += c;
    }

    client.stop();
    Serial.println("Response received!");
    return responseData;
}

void parseMetarData(String data) {
    Serial.println("Parsing Metar data...");
    // Define starting tags for each relevant field
    const String TAG_METAR = "<METAR>";
    const String TAG_FLIGHT_CATEGORY = "<flight_category>";

    // Find the start of the first METAR entry
    int startIndex = data.indexOf(TAG_METAR);
    if (startIndex == -1) {
        Serial.println("No METAR data found");
        return;
    }

    // Find the end of the first METAR entry
    int endIndex = data.indexOf("</METAR>", startIndex);
    if (endIndex == -1) {
        Serial.println("Invalid METAR data format");
        return;
    }

    // Extract the METAR data for parsing
    String metarData = data.substring(startIndex, endIndex);

    // Find the flight category tag
    int categoryStart = metarData.indexOf(TAG_FLIGHT_CATEGORY);
    if (categoryStart == -1) {
        Serial.println("Flight category not found in METAR data");
        return;
    }

    // Find the end of the flight category tag
    int categoryEnd = metarData.indexOf("</flight_category>", categoryStart);
    if (categoryEnd == -1) {
        Serial.println("Invalid flight category data format");
        return;
    }

    // Extract the flight category value
    String flightCategory = metarData.substring(categoryStart + TAG_FLIGHT_CATEGORY.length(), categoryEnd);

    // Process the flight category and set color accordingly
    setColor(flightCategory);
}

void setColor(String flightCategory) {
    Serial.print("Setting color for flight category: ");
    Serial.println(flightCategory);

    CRGB color;
    if (flightCategory.equals("VFR")) {
        color = CRGB::Green;
    } else if (flightCategory.equals("MVFR")) {
        color = CRGB::Blue;
    } else if (flightCategory.equals("IFR")) {
        color = CRGB::Red;
    } else if (flightCategory.equals("LIFR")) {
        color = CRGB::Magenta;
    } else {
        // Unknown category, set to black or handle differently
        color = CRGB::Black;
    }

    // Set color for all LEDs
    fill_solid(leds, NUM_AIRPORTS, color);
    FastLED.show();
}


void updateBrightness() {
  Serial.println("Updating brightness...");
    int potValue = analogRead(POT_PIN);
    int brightness = map(potValue, 0, 1023, 0, 255);
    brightness = 25;
    FastLED.setBrightness(brightness);
}

bool isReprogramButtonPressed() {
  return digitalRead(REPROGRAM_BUTTON_PIN) == LOW;
}

void enterReprogramMode() {
    Serial.println("Reprogram mode activated.");
}
