// 1: Organize includes
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
#include <WiFiManager.h>
#include "Config.h"
#include "Utilities.h"
#include "MetarFunctions.h"
#include "WifiFunctions.h"

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
