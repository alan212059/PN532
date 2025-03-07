#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define SDA_PIN 4  // Change as per your setup
#define SCL_PIN 5  // Change as per your setup

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

void setup() {
    Serial.begin(115200);
    Serial.println("NFC Writer Initialized");
    Wire.begin(SDA_PIN, SCL_PIN);
    nfc.begin();
    nfc.SAMConfig();  // Enable NFC reading mode
}

void loop() {
    Serial.println("Place an NFC tag to write...");
    
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};  // Buffer for UID
    uint8_t uidLength;                      // Length of UID

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        Serial.println("Tag detected!");

        // Default key for MIFARE Classic
        uint8_t keyA[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        // Authenticate block 4
        if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyA)) {
            Serial.println("Authentication successful!");

            // Sample text to write
            uint8_t data[] = {'2', '3', '0', '0', '2', '1', '2', '3', '6', ' ', ' ', ' ', ' ', ' ', ' ', ' '};


            // Write data to block 4
            if (nfc.mifareclassic_WriteDataBlock(4, data)) {
                Serial.println("Write successful!");
            } else {
                Serial.println("Write failed!");
            }
        } else {
            Serial.println("Authentication failed!");
        }
    }
    delay(2000);
}
