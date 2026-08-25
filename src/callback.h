#ifndef _CALLBACK_H_
#define _CALLBACK_H_

void callback(char* topic, byte* message, unsigned int length) {
  Serial.println("[MQTT] Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Door status
  if(String(topic) == door_status_request) {
    doorStatus();
  } else 
  if (String(topic) == door_permit && messageTemp == "1") {
    lockOpen();
  } else
  if (String(topic) == reader_read_request) {
    startFastInventoryTest(1);
  }
}

#endif
