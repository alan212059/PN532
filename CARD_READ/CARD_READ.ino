#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define SDA_PIN 4
#define SCL_PIN 5

const char* ssid = "Enlor";
const char* password = "houseofenlor";
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbzw99zCr4i00NUqY5lvPifSq3EAuuB42JdCNtBNGGpWrIn689WIRgWnR2Sc3kZSOokdSA/exec";

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    nfc.begin();
    nfc.SAMConfig();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi");
}

void loop() {
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
    uint8_t uidLength;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        uint8_t keyA[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyA)) {
            uint8_t readData[16];
            if (nfc.mifareclassic_ReadDataBlock(4, readData)) {
                String dataString = "";
                for (uint8_t i = 0; i < 16; i++) {
                    if (readData[i] >= 32 && readData[i] <= 126) {
                        dataString += (char)readData[i];
                    } else {
                        // Handle non-printable characters if needed
                    }
                }
                dataString.trim(); // Remove any trailing whitespace
                sendToGoogleSheets(uid, uidLength, dataString);
            }
        }
        delay(2000); // Prevent rapid repeated reads
    }
}

void sendToGoogleSheets(uint8_t* uid, uint8_t uidLength, String data) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client; // Use WiFiClientSecure for HTTPS
        HTTPClient http;

        // Convert UID to hex string
        String uidString;
        for (uint8_t i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) uidString += "0";
            uidString += String(uid[i], HEX);
        }

        // URL-encode the data
        String encodedData = urlEncode(data);

        // Prepare POST data
        String postData = "uid=" + uidString + "&data=" + encodedData;

        // Configure HTTPS client (ignore SSL certificate validation for simplicity)
        client.setInsecure(); // Only if you trust the host

        if (http.begin(client, googleScriptURL)) {
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            
            int httpCode = http.POST(postData);
            if (httpCode > 0) {
                Serial.printf("HTTP Code: %d\n", httpCode);
                if (httpCode == HTTP_CODE_OK) {
                    String response = http.getString();
                    Serial.println(response);
                }
            } else {
                Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
        } else {
            Serial.println("Failed to connect to script URL.");
        }
    }
}

// Helper function for URL encoding
String urlEncode(String str) {
    String encoded = "";
    char c;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            encoded += "%";
            char hex[3];
            sprintf(hex, "%02X", (unsigned char)c); // Convert to uppercase hex
            encoded += hex;
        }
    }
    return encoded;
}