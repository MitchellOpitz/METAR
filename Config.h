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

int timeout = 120;
String airports = "";
WiFiManager wifiManager;
CRGB leds[NUM_AIRPORTS];
String data;

// Possibly unused
#define BRIGHT_PIN A0
