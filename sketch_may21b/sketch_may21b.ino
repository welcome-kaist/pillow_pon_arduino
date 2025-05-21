#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11); // HC-06 TX, RX

unsigned long lastSend = 0;
const unsigned long sendInterval = 2000;  // 2초마다 전송

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);  // HC-06 기본 속도
}

void loop() {
  unsigned long now = millis();

  if (now - lastSend >= sendInterval) {
    sendDummyJson();
    lastSend = now;
  }
}

void sendDummyJson() {
  String json = "{";
  json += "\"temp\":24.5,";
  json += "\"humid\":60.0,";
  json += "\"cds\":350,";
  json += "\"sound\":480,";
  json += "\"pressure\":620,";
  json += "\"motion\":1,";
  json += "\"acc1\":{\"x\":-132,\"y\":789,\"z\":16384},";
  json += "\"acc2\":{\"x\":128,\"y\":730,\"z\":16200}";
  json += "}";

  Serial.println(json);      // 시리얼 모니터 출력
  BTSerial.println(json);    // 블루투스로 전송
}
