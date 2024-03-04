#ifndef WIFI_FUNCTIONS_H
#define WIFI_FUNCTIONS_H

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include "Config.h"
#include "Utilities.h"

// Declare wifiManager as extern
extern WiFiManager wifiManager;

// Declare airports variable as extern
extern String airports;

// Function prototypes
void configureWifi();
void connectToAccessPoint();

// Function implementations
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
