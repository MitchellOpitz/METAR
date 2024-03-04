const int NUM_LEDS = 25;
const int LOOP_INTERVAL = 300000;
const int TRIGGER_PIN = D4;
const int DATA_PIN = D2;
const int POT_PIN = A0;
const int REPROGRAM_BUTTON_PIN = D3;
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define SERVER "aviationweather.gov"
#define BASE_URI "/api/data/dataserver.php?dataSource=metars&requestType=retrieve&format=xml&hoursBeforeNow=3&mostRecentForEachStation=constraint&stationString="

int timeout = 120;
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_LEDS];
String data;

// Possibly unused
#define BRIGHT_PIN A0
