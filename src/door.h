#ifndef _DOOR_H_
#define _DOOR_H_

bool useLockSecurity = true;
bool doorIsOpen = false;
bool warningIsOn = false;
bool alarmIsOn = false;

int32_t lockOpened_ms = -10001;
uint32_t doorOpened_ms = 0;
uint32_t warningStarted_ms = 0;
uint32_t alarmStarted_ms = 0;
constexpr uint32_t TIME_TO_OPEN_MS = 10000;
constexpr uint32_t TIME_TO_CHOOSE_MS = 20000;
constexpr uint32_t TIME_TO_WARN_MS = 10000;
constexpr uint8_t PIXEL_COUNT = 3;
constexpr neoPixelType STRIP_FORMAT = NEO_GRBW + NEO_KHZ800;


Adafruit_NeoPixel strip(PIXEL_COUNT, LIGHT, STRIP_FORMAT);

void doorStatus() {
    Serial.println("[MQTT] Request to check the door status");
    if(digitalRead(LOCK_SWITCH) == HIGH) {
        Serial.println("[LOCK] Door is open");
        publishMessage(door_status.c_str(), "1", false, true);
    } else {
        Serial.println("[LOCK] Door is closed");
        publishMessage(door_status.c_str(), "0", false, true);
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

void doorOpen() {
    doorIsOpen = true;
    doorOpened_ms = millis();
    useLockSecurity = false;
    digitalWrite(LOCK, LOW);
    Serial.print("Ta fukncja chyba wykonuje sie wiele razy podczas otwartych drzwi");
}

void lockSecurity() {
    if(digitalRead(LOCK_SWITCH) == LOW && (millis() - lockOpened_ms > TIME_TO_OPEN_MS || doorIsOpen) ) {
        digitalWrite(LOCK, LOW);
        digitalWrite(BUZZER, LOW);
        strip.clear();
        strip.show();
        doorIsOpen = false;
        return;
    }

    if (digitalRead(LOCK_SWITCH) == HIGH && millis() - lockOpened_ms < TIME_TO_OPEN_MS) {
        if(!doorIsOpen) {
            doorOpen();
        }
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip.setPixelColor(pixel, strip.Color(0, 0, 0, 255));
        }
        strip.show();
        return;
    }

    if (digitalRead(LOCK_SWITCH) == HIGH && millis() - lockOpened_ms > TIME_TO_OPEN_MS) {
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip.setPixelColor(pixel, strip.Color(255, 0, 0, 0));
        }
        strip.show();

    }
    
}

void doorSecurity() {
    if (digitalRead(LOCK_SWITCH) == HIGH && millis() - doorOpened_ms > TIME_TO_CHOOSE_MS && !warningIsOn) {
        warningIsOn = true;
        warningStarted_ms = millis();
        for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip.setPixelColor(pixel, strip.Color(255, 0, 255, 0));
        }
        strip.show();
        Serial.print("To też powinno wydarzyć sie raz");
        return;
    }

    if(digitalRead(LOCK_SWITCH) == HIGH && millis() - warningStarted_ms > TIME_TO_WARN_MS && warningIsOn) {
        alarmIsOn = true;
        digitalWrite(BUZZER, HIGH);
         for (uint8_t pixel = 0; pixel < PIXEL_COUNT; ++pixel) {
            strip.setPixelColor(pixel, strip.Color(255, 128, 128, 0));
        }
        strip.show();
        return;
    }

    if(digitalRead(LOCK_SWITCH) == LOW) {
        Serial.print("Chyba tu nie dochodzi?");
        useLockSecurity = true;
    }
}

#endif