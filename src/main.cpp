#include <Arduino.h>
#include <time.h>
// Pin definitions
#include "pins.h"
// Ethernet and MQTT functions
#include "connection.h"

#include "door.h"



void setup() {
  Serial.begin(115200);
  //tu sie czasem czeka lae w sumie nie wiem po co
  startEthernet();

  espClient.setCACert(root_ca);
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  pinMode(LOCK_SWITCH, INPUT_PULLUP);

}

void loop() {
  if (!client.connected())
    reconnect();
  client.loop();

}