#ifndef _CONNECTION_H_
#define _CONNECTION_H_

constexpr const char* ETHERNET_HOSTNAME = "esp32-p4";
constexpr uint32_t ETHERNET_DHCP_TIMEOUT_MS = 15000;

constexpr const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;

uint32_t epochTime;
uint32_t lastTimestamp_ms = 0;
constexpr uint16_t TIMESTAMP_INTERVAL_MS = 1000;

WiFiClientSecure espClient;

PubSubClient client(espClient);

// MQTT Message buffer
uint32_t lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];

const char* clientID = "ESP32-P4-ETH";

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

unsigned long getUnixTime(){
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);

  return now;
}

// Connect to MQTT broker
void reconnect() {
  // loop until connected
  while(!client.connected()) {
    Serial.print("\n[MQTT] Connecting to broker");
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.print("\n[MQTT] Connected");
    }
    else {
      Serial.print("\n[MQTT] Failed, rc=");
      Serial.print(client.state());
      Serial.print("\n[MQTT] Retrying in 2 seconds");
      delay(2000);
    }
  }
  client.subscribe("test/topic");
  client.subscribe("asps/asp0/asm3/door/status/request");
  client.subscribe("asps/asp0/asm3/door/permit");
  client.subscribe("asps/asp0/asm3/reader/read/request");
}

void publishMessage(const char* topic, String payload, boolean retained, boolean print) {
  if(client.publish(topic, payload.c_str(), retained) && print) {
    Serial.print("\n[MQTT] Message published [" + String(topic) + "]: " + payload);
  }
}

void timestamp() {
  if(millis() - lastTimestamp_ms >= TIMESTAMP_INTERVAL_MS)
  {
    publishMessage("asps/asp0/asm3/timestamp", String(getUnixTime()), true, false);
    lastTimestamp_ms = millis();
  }
}

#endif
