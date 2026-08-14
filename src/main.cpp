#include <Arduino.h>
#include <time.h>
#include "pins.h"

#include <ETH.h>

constexpr const char* ETHERNET_HOSTNAME = "esp32-p4";
constexpr uint32_t ETHERNET_DHCP_TIMEOUT_MS = 15000;

// Flag
bool isEthernetOnline = false;

void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("[ETH] ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname(ETHERNET_HOSTNAME);
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: Serial.println("[ETH] Connected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("[ETH] Got IP");
      Serial.println(ETH);
      isEthernetOnline = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("[ETH] Lost IP");
      isEthernetOnline = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[ETH] Disconnected");
      isEthernetOnline = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("[ETH] Stopped");
      isEthernetOnline = false;
      break;
    default: break;
  }
}

bool startEthernet() {
  Network.onEvent(onEvent);

  Serial.println("[ETH] Starting Ethernet");
  if (!ETH.begin()) {
    Serial.println("[ETH] Ethernet failed to start");
    return false;
  }

  const uint32_t connectionStartMs = millis();
  // biblioteka ETH działa chyba na zasadzie przerwań dlatego ten while ma sens
  while (!isEthernetOnline && connectionStartMs < ETHERNET_DHCP_TIMEOUT_MS) {
    delay(100);
  }

  if (!isEthernetOnline) {
    Serial.println("[ETH] Couldn't connect with DHCP");
    return false;
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  //tu sie czasem czeka lae w sumie nie wiem po co
  startEthernet();

}

void loop() {

}