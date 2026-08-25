#include <Arduino.h>
#include <time.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "../../.secrets/secret.h"
#include "topics.h"
// Pin definitions
#include "pins.h"
// Ethernet, time and MQTT connection
#include "connection.h"
// Lock and door functions
#include "door.h"
// Reader functions
#include "reader.h"
// Handling MQTT requests
#include "callback.h"



void setup() {
  Serial.begin(115200);
  pinMode(LOCK, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LOCK_SWITCH, INPUT_PULLUP);

  RfidSerial.setRxBufferSize(RFID_RX_BUFFER_SIZE);
  RfidSerial.begin(RFID_BAUD_RATE, SERIAL_8N1, READER_RX, READER_TX);

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

  requestReaderInfo();

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

  flashLed();

  handleReaderRequest();

  timestamp();

}