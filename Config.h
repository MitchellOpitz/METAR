#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
const int TRIGGER_PIN = D4;
const int DATA_PIN = D2;
const int REPROGRAM_BUTTON_PIN = D3;
const int POT_PIN = A0;

// LED Configuration
const int NUM_LEDS = 25;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

// Network Configuration
const char* SERVER = "aviationweather.gov";
const char* BASE_URI = "/api/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString=";

// Timing Configuration
const int LOOP_INTERVAL = 300000; // 300000 milliseconds = 5 minutes

#endif
