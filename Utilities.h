#ifndef UTILITIES_H
#define UTILITIES_H

#include <EEPROM.h>
#include <FastLED.h>
#include "WifiFunctions.h"

extern CRGB leds[];

void writeStringToEEPROM(char add, String data) {
    int _size = data.length();
    for (int i = 0; i < _size; i++) {
        EEPROM.write(add + i, data[i]);
    }
    EEPROM.write(add + _size, '\0');
    EEPROM.commit();
}

String readStringFromEEPROM(char add) {
    const int maxLength = 100;
    char data[maxLength + 1];
    int len = 0;
    char currentChar;

    do {
        currentChar = EEPROM.read(add + len);
        data[len] = currentChar;
        len++;
    } while (currentChar != '\0' && len < maxLength);

    data[len] = '\0';

    return String(data);
}

void initializePins() {    
    Serial.println("Initializing pins...");
    pinMode(REPROGRAM_BUTTON_PIN, INPUT_PULLUP);    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void changeLEDColor (CRGB color) {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}

void initializeLeds() {    
    Serial.println("Initializing LEDs...");
    int BRIGHTNESS = 5;
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    changeLEDColor(CRGB::Orange);
}

bool isReprogramButtonPressed() {
    return digitalRead(REPROGRAM_BUTTON_PIN) == LOW;
}

void enterReprogramMode() {
    Serial.println("Reprogram mode activated.");
    connectToWifiConfigPortal();
}

void updateBrightness() {
    Serial.println("Updating brightness...");
    int potValue = analogRead(POT_PIN);
    int brightness = map(potValue, 0, 1023, 0, 255);
    brightness = 25;
    FastLED.setBrightness(brightness);
}

#endif
