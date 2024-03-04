#ifndef GLOBALS_H
#define GLOBALS_H

#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <WiFiManager.h>
#include "Config.h"

extern int timeout;
extern String airports;
extern WiFiManager wifiManager;
extern CRGB leds[];
extern String data;

// Possibly unused
extern const int BRIGHT_PIN;

// Define global variables
int timeout = 120;
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_LEDS];
String data;

// Possibly unused
const int BRIGHT_PIN = A0;

#endif
