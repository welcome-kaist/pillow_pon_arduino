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
SoftwareSerial BTSerial(10, 11);
MPU6050 mpu1(0x68);  // ADO → GND
MPU6050 mpu2(0x69);  // ADO → VCC

String inputCommand = "";
unsigned long lastEnvSend = 0;
const unsigned long ENV_INTERVAL = 10UL * 60UL * 1000UL;  // 10분

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
  unsigned long now = millis();

  // 블루투스 명령 수신
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

  // 센서 읽기
  int sound = analogRead(SOUND_PIN);
  int pressure = analogRead(PRESSURE_PIN);
  int motion = digitalRead(PIR_PIN);

  int16_t ax1, ay1, az1, ax2, ay2, az2;
  mpu1.getAcceleration(&ax1, &ay1, &az1);
  mpu2.getAcceleration(&ax2, &ay2, &az2);

  // 실시간 JSON 전송
  String liveJson = "{";
  liveJson += "\"sound\":" + String(sound) + ",";
  liveJson += "\"pressure\":" + String(pressure) + ",";
  liveJson += "\"motion\":" + String(motion) + ",";
  liveJson += "\"acc1\":{\"x\":" + String(ax1) + ",\"y\":" + String(ay1) + ",\"z\":" + String(az1) + "},";
  liveJson += "\"acc2\":{\"x\":" + String(ax2) + ",\"y\":" + String(ay2) + ",\"z\":" + String(az2) + "}";
  liveJson += "}";

  Serial.println(liveJson);
  BTSerial.println(liveJson);

  // 환경 데이터는 10분마다 전송
  if (now - lastEnvSend >= ENV_INTERVAL) {
    float temp = dht.readTemperature();
    float humid = dht.readHumidity();
    int cds = analogRead(CDS_PIN);

    String envJson = "{";
    envJson += "\"temp\":" + String(temp, 1) + ",";
    envJson += "\"humid\":" + String(humid, 1) + ",";
    envJson += "\"cds\":" + String(cds);
    envJson += "}";

    Serial.println(envJson);
    BTSerial.println(envJson);

    lastEnvSend = now;
  }

  delay(2000);  // 실시간 데이터 주기
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
