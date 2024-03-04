// 1: Organize includes
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
#include <WiFiManager.h>
#include "Config.h"
#include "Utilities.h"
#include "MetarFunctions.h"

using namespace std;

int timeout = 120;
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_LEDS];
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
        changeLEDColor(CRGB::Purple);
    } else {
        Serial.println("Failed to connect to WiFi or hit timeout.");
        changeLEDColor(CRGB::Orange);
    }
}
