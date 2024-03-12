#include "Globals.h"
#include "Utilities.h"
#include "MetarFunctions.h"
#include "WifiFunctions.h"

void setup() {
    Serial.begin(115200);
    EEPROM.begin(512);

    initializePins();
    initializeLeds();
    configureWifi();
}

void loop() {  
    updateBrightness();
    if (isReprogramButtonPressed()) {
        enterReprogramMode();
    } else {
      if(millis() - lastRunTime >= LOOP_INTERVAL) {
        connectToAccessPoint();
        readAirportData();
        String metarData = retrieveMetarData(airports);
        if(!metarData.isEmpty()) {
            parseMetarData(metarData);
        }
        lastRunTime = millis();
      }
    }
}
