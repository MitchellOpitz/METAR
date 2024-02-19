void connectToWifi() {
    wifiManager.setConfigPortalBlocking(true);  // Disables WifI configuration portal.  May not be needed.
    if (!wifiManager.autoConnect("MetarWiFi")) {
        Serial.println("Failed to connect to WiFi or hit timeout.");
    } else {
        Serial.println("Connected to WiFi.");
    }
}

void configureWiFiManager() {
    wifiManager.setDebugOutput(false);
    wifiManager.resetSettings();
}

void configureWiFi() {
    WiFi.mode(WIFI_STA);
}

void setupWiFiManager() {
    WiFiManagerParameter custom_text_box("ICAO", "Enter Your Airport Here", "", 4);
    wifiManager.addParameter(&custom_text_box);
}

void setupCustomParameter() {
    String data = custom_text_box.getValue();
    if (data != "") {
        writeStringToEEPROM(10, data);
    }
}

void indicateWifiStatus() {
    fill_solid(leds, NUM_AIRPORTS, CRGB::Orange);
    FastLED.show();
}
