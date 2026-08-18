#ifndef _DOOR_H_
#define _DOOR_H_


uint32_t lockOpened_ms = 0;
constexpr uint32_t TIME_TO_OPEN_MS = 10000;
constexpr uint8_t PIXEL_COUNT = 3;
constexpr neoPixelType STRIP_FORMAT = NEO_GRBW + NEO_KHZ800;


Adafruit_NeoPixel strip(PIXEL_COUNT, LIGHT, STRIP_FORMAT);

void doorStatus() {
    Serial.println("[MQTT] Request to check the door status");
    if(digitalRead(LOCK_SWITCH) == HIGH) {
        Serial.println("[LOCK] Door is open");
        publishMessage("asps/asp0/asm3/door/status", "1", false, true);
    } else {
        Serial.println("[LOCK] Door is closed");
        publishMessage("asps/asp0/asm3/door/status", "0", false, true);
    }
}

void lockOpen() {
    lockOpened_ms = millis();
    digitalWrite(LOCK, HIGH);
    for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
        strip.setPixelColor(pixel, strip.Color(0, 255, 0, 0));
    }
    strip.show();
}

void lockSecurity() {
    if(digitalRead(LOCK_SWITCH) == LOW && millis() - lockOpened_ms >= TIME_TO_OPEN_MS) {
        digitalWrite(LOCK, LOW);
        strip.clear();
        strip.show();
    }
}

#endif