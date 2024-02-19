// 1: Organize includes
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <vector>
#include <EEPROM.h>
#include <WiFiManager.h>
#include "Metar_Wifi.ino"
#include "Metar_EEPROM.ino"
#include "Metar_DataHandling.ino"

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
    String metarData = retrieveMetarData(airports);
    parseMetarData();
    doColor();
    handleDelay();
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
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}
