#ifndef UTILITIES_H
#define UTILITIES_H

#include <EEPROM.h>

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

bool isReprogramButtonPressed() {
  return digitalRead(REPROGRAM_BUTTON_PIN) == LOW;
}

void enterReprogramMode() {
    Serial.println("Reprogram mode activated.");
}

#endif
