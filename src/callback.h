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
  if(String(topic) == "asps/asp0/asm3/door/status/request") {
    doorStatus();
  } else 
  if (String(topic) == "asps/asp0/asm3/door/permit" && messageTemp == "1") {
    lockOpen();
  }
}

#endif