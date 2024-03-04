#ifndef METAR_FUNCTIONS_H
#define METAR_FUNCTIONS_H

extern const char* SERVER;
extern const char* BASE_URI;

String retrieveMetarData(String airports) {
    Serial.println("Retrieving Metar data...");
    
    // Establish a secure HTTPS connection to the server
    BearSSL::WiFiClientSecure client;
    client.setInsecure(); // For development/testing only; use proper certificates in production

    if (!client.connect(SERVER, 443)) {
        Serial.println("Connection failed!");
        return ""; // Indicate failure
    }

    // Construct the HTTP request header with appropriate formatting
    String request = "GET " + String(BASE_URI) + airports + " HTTP/1.1\r\n" +
                    "Host: " + String(SERVER) + "\r\n" +
                    "User-Agent: LED Sectional Client\r\n" +
                    "Connection: close\r\n\r\n";

    // Send the HTTP request
    Serial.println("Sending request...");
    Serial.println(request);
    client.print(request);

    // Set a reasonable timeout to prevent indefinite waiting
    unsigned long timeout = millis();
    const unsigned long maxTimeout = 5000; // 5 seconds

    // Receive and process the response
    String responseData = "";
    while (client.connected() && !client.available()) {
        if (millis() - timeout > maxTimeout) {
            Serial.println("Timeout waiting for response");
            client.stop();
            return ""; // Indicate timeout
        }
    }

    // Read response data
    while (client.available()) {
        char c = client.read();
        responseData += c;
    }

    client.stop();
    Serial.println("Response received!");
    return responseData;
}

void parseMetarData(String data) {
    Serial.println("Parsing Metar data...");
    // Define starting tags for each relevant field
    const String TAG_METAR = "<METAR>";
    const String TAG_FLIGHT_CATEGORY = "<flight_category>";

    // Find the start of the first METAR entry
    int startIndex = data.indexOf(TAG_METAR);
    if (startIndex == -1) {
        Serial.println("No METAR data found");
        return;
    }

    // Find the end of the first METAR entry
    int endIndex = data.indexOf("</METAR>", startIndex);
    if (endIndex == -1) {
        Serial.println("Invalid METAR data format");
        return;
    }

    // Extract the METAR data for parsing
    String metarData = data.substring(startIndex, endIndex);

    // Find the flight category tag
    int categoryStart = metarData.indexOf(TAG_FLIGHT_CATEGORY);
    if (categoryStart == -1) {
        Serial.println("Flight category not found in METAR data");
        return;
    }

    // Find the end of the flight category tag
    int categoryEnd = metarData.indexOf("</flight_category>", categoryStart);
    if (categoryEnd == -1) {
        Serial.println("Invalid flight category data format");
        return;
    }

    // Extract the flight category value
    String flightCategory = metarData.substring(categoryStart + TAG_FLIGHT_CATEGORY.length(), categoryEnd);

    // Process the flight category and set color accordingly
    CRGB color;
    if (flightCategory.equals("VFR")) {
        color = CRGB::Green;
    } else if (flightCategory.equals("MVFR")) {
        color = CRGB::Blue;
    } else if (flightCategory.equals("IFR")) {
        color = CRGB::Red;
    } else if (flightCategory.equals("LIFR")) {
        color = CRGB::Magenta;
    } else {
        // Unknown category, set to black or handle differently
        color = CRGB::Black;
    }

    changeLEDColor(color);
}

#endif
