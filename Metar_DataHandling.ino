 void readAirportData() {
    airports = readStringFromEEPROM(10);
    Serial.print("Airport: ");
    Serial.println(airports);
}

String retrieveMetarData(String airports) {
    BearSSL::WiFiClientSecure client;
    client.setInsecure();

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        client.stop();
        return ""; // Return an empty string indicating failure
    }

    // Construct and send HTTP request
    String request = "GET " + String(BASE_URI) + airports + " HTTP/1.1\r\n" +
                     "Host: " + String(SERVER) + "\r\n" +
                     "User-Agent: LED Sectional Client\r\n" +
                     "Connection: close\r\n\r\n";
    client.print(request);

    // Wait for response
    unsigned long timeout = millis();
    while (!client.available()) {
        if (millis() - timeout > 5000) {
            Serial.println("Timeout waiting for response");
            client.stop();
            return ""; // Return an empty string indicating timeout
        }
    }

    // Read and process response
    String responseData = "";
    while (client.available()) {
        // Read response data
        String line = client.readStringUntil('\r');
        responseData += line; // Append each line to the response data
    }

    client.stop();
    return responseData; // Return the received data
}

void parseMetarData(String data) {
    String currentAirport = "";
    String currentCondition = "";
    String currentWind = "";
    String currentGusts = "";
    String currentWxstring = "";

    bool readingAirport = false;
    bool readingWind = false;
    bool readingGusts = false;
    bool readingCondition = false;
    bool readingWxstring = false;

    for (int i = 0; i < data.length(); i++) {
        char c = data.charAt(i);

        if (c == '\n') {
            currentLine = "";
            continue;
        }

        currentLine += c;

        if (currentLine.endsWith("<station_id>")) {
            if (!led.empty()) {
                for (auto it = led.begin(); it != led.end(); ++it) {
                    doColor(currentAirport, *it, currentWind.toInt(), currentGusts.toInt(), currentCondition, currentWxstring);
                }
                led.clear();
            }
            currentAirport = "";
            readingAirport = true;
            currentCondition = "";
            currentWind = "";
            currentGusts = "";
            currentWxstring = "";
        } else if (readingAirport) {
            if (!currentLine.endsWith("<")) {
                currentAirport += c;
            } else {
                readingAirport = false;
                for (unsigned short int i = 0; i < NUM_AIRPORTS; i++) {
                    if (airports == currentAirport) {
                        led.push_back(i);
                    }
                }
            }
        } else if (currentLine.endsWith("<wind_speed_kt>")) {
            readingWind = true;
        } else if (readingWind) {
            if (!currentLine.endsWith("<")) {
                currentWind += c;
            } else {
                readingWind = false;
            }
        } else if (currentLine.endsWith("<wind_gust_kt>")) {
            readingGusts = true;
        } else if (readingGusts) {
            if (!currentLine.endsWith("<")) {
                currentGusts += c;
            } else {
                readingGusts = false;
            }
        } else if (currentLine.endsWith("<flight_category>")) {
            readingCondition = true;
        } else if (readingCondition) {
            if (!currentLine.endsWith("<")) {
                currentCondition += c;
            } else {
                readingCondition = false;
            }
        } else if (currentLine.endsWith("<wx_string>")) {
            readingWxstring = true;
        } else if (readingWxstring) {
            if (!currentLine.endsWith("<")) {
                currentWxstring += c;
            } else {
                readingWxstring = false;
            }
        }
    }
}
