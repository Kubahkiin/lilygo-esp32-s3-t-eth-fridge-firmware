#include <Arduino.h>
#include <time.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "../../.secrets/secret.h"
// Pin definitions
#include "pins.h"
// Ethernet, time and MQTT connection
#include "connection.h"
// Lock and door functions
#include "door.h"
// Handling MQTT requests
#include "callback.h"



void setup() {
  Serial.begin(115200);
  //tu sie czasem czeka lae w sumie nie wiem po co
  pinMode(LOCK, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LOCK_SWITCH, INPUT_PULLUP);

  strip.begin();
  strip.setBrightness(32);
  strip.clear();
  strip.show();

  startEthernet();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  epochTime = getUnixTime();
  Serial.print("[NTP] Epoch Time: ");
  Serial.println(epochTime);
  lastTimestamp_ms = millis();
  espClient.setCACert(root_ca);
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();
 

  if(useLockSecurity) {
    lockSecurity();
  } else {
    doorSecurity();
  }

  timestamp();

}