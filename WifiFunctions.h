#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include <WiFiManager.h>
#include "Utilities.h"

extern WiFiManager wifiManager;
extern String airports;

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

    connectToWifiConfigPortal();
}

void connectToWifiConfigPortal(){
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

#endif
