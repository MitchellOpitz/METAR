#ifndef GLOBALS_H
#define GLOBALS_H

#include <FastLED.h>
#include <WiFiManager.h>
#include "Config.h"

extern String airports;
extern WiFiManager wifiManager;
extern CRGB leds[];
extern String data;

// Define global variables
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_LEDS];
String data;

#endif
