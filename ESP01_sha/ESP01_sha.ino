#include <Arduino.h>
#include <Hash.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
    // usage as ptr
    // SHA1:a94a8fe5ccb19ba61c4c0873d391e987982fbbd3
    uint8_t hash[20];
    sha1("abc", hash);

    Serial.print("SHA1: ");
    for(uint16_t i = 0; i < 20; i++) {
        Serial.printf("%02x", hash[i]);
    }
    Serial.println();

    delay(1000);
}

