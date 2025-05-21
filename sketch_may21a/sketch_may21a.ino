#include <Wire.h>
#include <MPU6050.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define CDS_PIN A0
#define SOUND_PIN A1
#define PRESSURE_PIN A2
#define PIR_PIN 3
#define VIBE1_PIN 9
#define VIBE2_PIN 5

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial BTSerial(10, 11);  // D10=RX, D11=TX
MPU6050 mpu1(0x68);  // ADO → GND
MPU6050 mpu2(0x69);  // ADO → VCC

String inputCommand = "";
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10UL * 60UL * 1000UL;  // 10분 = 600,000ms

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  dht.begin();
  Wire.begin();
  mpu1.initialize();
  mpu2.initialize();

  pinMode(PIR_PIN, INPUT);
  pinMode(VIBE1_PIN, OUTPUT);
  pinMode(VIBE2_PIN, OUTPUT);
  digitalWrite(VIBE1_PIN, LOW);
  digitalWrite(VIBE2_PIN, LOW);
}

void loop() {
  // 블루투스 명령 수신 (실시간)
  while (BTSerial.available()) {
    char c = BTSerial.read();
    if (c == '\n') {
      inputCommand.trim();
      handleCommand(inputCommand);
      inputCommand = "";
    } else {
      inputCommand += c;
    }
  }

  // JSON 데이터 전송 타이머
  if (millis() - lastSendTime >= sendInterval) {
    sendJson();
    lastSendTime = millis();
  }
}

void handleCommand(String command) {
  if (command == "VIBRATE:ALL") {
    digitalWrite(VIBE1_PIN, HIGH);
    digitalWrite(VIBE2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIBE1_PIN, LOW);
    digitalWrite(VIBE2_PIN, LOW);
  } 
  else if (command == "VIBRATE:SPOT1") {
    digitalWrite(VIBE1_PIN, HIGH);
    delay(1000);
    digitalWrite(VIBE1_PIN, LOW);
  } 
  else if (command == "VIBRATE:SPOT2") {
    digitalWrite(VIBE2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIBE2_PIN, LOW);
  }
}

void sendJson() {
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int cds = analogRead(CDS_PIN);
  int sound = analogRead(SOUND_PIN);
  int pressure = analogRead(PRESSURE_PIN);
  int motion = digitalRead(PIR_PIN);

  int16_t ax1, ay1, az1, ax2, ay2, az2;
  mpu1.getAcceleration(&ax1, &ay1, &az1);
  mpu2.getAcceleration(&ax2, &ay2, &az2);

  String json = "{";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"humid\":" + String(humid, 1) + ",";
  json += "\"cds\":" + String(cds) + ",";
  json += "\"sound\":" + String(sound) + ",";
  json += "\"pressure\":" + String(pressure) + ",";
  json += "\"motion\":" + String(motion) + ",";
  json += "\"acc1\":{\"x\":" + String(ax1) + ",\"y\":" + String(ay1) + ",\"z\":" + String(az1) + "},";
  json += "\"acc2\":{\"x\":" + String(ax2) + ",\"y\":" + String(ay2) + ",\"z\":" + String(az2) + "}";
  json += "}";

  Serial.println(json);
  BTSerial.println(json);
}
