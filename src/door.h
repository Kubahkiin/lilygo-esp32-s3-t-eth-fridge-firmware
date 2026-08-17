#ifndef _DOOR_H_
#define _DOOR_H_

void doorStatus() {
    Serial.println("[MQTT] Request to check the door status");
    if(digitalRead(LOCK_SWITCH) == HIGH) {
        Serial.println("[LOCK] Door is open");
        publishMessage("asps/asp0/asm3/door/status", "1", false);
    } else {
        Serial.println("[LOCK] Door is closed");
        publishMessage("asps/asp0/asm3/door/status", "0", false);
    }
}

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
  }
}

#endif