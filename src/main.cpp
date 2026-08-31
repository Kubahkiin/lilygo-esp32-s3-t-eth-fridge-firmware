#include <Arduino.h>
#include <time.h>
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include <ETH.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
// Informacje poufne
#include "../../.secrets/secret.h"
// Zmienne z łańcuchami topiców
#include "topics.h"
// Definicje pinów 
#include "pins.h"
// Połączenie z Ethernetem, serwerem NTP i brokerem MQTT
#include "connection.h"
// Obsługa elektrozamka i monitorowanie stanu drzwi
#include "door.h"
// Obsługa czytnika
#include "reader.h"
// Obsługa żądań z serwera MQTT
#include "callback.h"


/**
 *  Główna funkcja konfiguracyjna programu.
 *  Wykonuje się zawsze po starcie systemu esp32
 */
void setup() {
  // Rozpoczęcie komunikacji esp-pc w celu wyświetlania komunikatów, prędkość 115200 baud
  Serial.begin(115200);
  // Ustawienie trybu pinów, żeby odeniść sie do nich w programie
  pinMode(LOCK, OUTPUT);
  //pinMode(BUZZER, OUTPUT);
  pinMode(LOCK_SWITCH, INPUT_PULLUP);
  // Rozpoczęcie komunikacji z czytnikiem przez UART
  RfidSerial.setRxBufferSize(RFID_RX_BUFFER_SIZE);
  RfidSerial.begin(RFID_BAUD_RATE, SERIAL_8N1, READER_RX, READER_TX);
  // Konfiguracja paska LED
  strip.begin();
  strip.setBrightness(32);
  strip.clear();
  strip.show();

  // Rozpoczęcie Ethernetu 
  startEthernet();

  // Konfiguracja połączenia z NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  epochTime = getUnixTime();
  Serial.print("[NTP] Epoch Time: ");
  Serial.println(epochTime);
  lastTimestamp_ms = millis();

  // Ustawienie certyfikatu 
  espClient.setCACert(root_ca);
  client.setServer(mqtt_broker, mqtt_port);

  // Jednorazowa konfiguracja czytnika przed pierwszą inwentaryzacją.
  startReaderConfiguration();

  // Wybranie funkcji obsługującej wiadomości/żądania MQTT
  client.setCallback(callback);
}

/**
 *  Główna pętla programu.
 *  Wykonuje się tak długo jak esp jest włączone
 */
void loop() {
  // Sprawdzenie czy jest połączenie
  if (!client.connected())
    // Jeśli nie, połącz ponownie
    reconnect();
  // Obsługa pętli MQTT (czy coś, jeszcze musze to sprawdzić)
  client.loop();
 
  // Sprawdzenie trybu monitorowania
  if(useLockSecurity) {
    // Monitorowanie drzwi poza permitem
    lockSecurity();
  } else {
    // Monitorowanie drzwi po pozwoleniu i otwarciu drzwi
    doorSecurity();
  }

  // Obsługa migających świateł
  flashLed();

  // Komunikacja z czytnikiem RFID
  handleReaderRequest();

  // Przesłanie timestamp
  timestamp();
}
