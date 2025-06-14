#include <DHT.h>
#include <SoftwareSerial.h>
#include <MPU6050.h>

// DHT11
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HC-06 Bluetooth
#define BT_RXD 8  // HC-06 TX → Arduino RX
#define BT_TXD 7  // Arduino TX → HC-06 RX (전압 분할!)
SoftwareSerial bluetooth(BT_RXD, BT_TXD);

// 센서 핀
#define CDS_PIN A0           // 조도 센서
#define SOUND_ANALOG_PIN A1  // 사운드 센서
#define PIR_PIN 4

// 진동 모터 핀
#define VIB1_PIN 5
#define VIB2_PIN 6

MPU6050 mpu;
#define FLEX_PIN A2

String command = "";  // Bluetooth 명령 수신 버퍼

void setup() {
  bluetooth.begin(9600);
  dht.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    bluetooth.println("{\"error\":\"MPU6050 connection failed\"}");
  }

  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
  pinMode(FLEX_PIN, INPUT);
}

void loop() {
  // 명령 수신 처리
  while (bluetooth.available()) {
    char c = bluetooth.read();
    if (c == '\n') {
      processCommand(command);
      command = "";
    } else {
      command += c;
    }
  }

  // 센서 읽기 및 전송
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  int cds = analogRead(CDS_PIN);
  int sound = analogRead(SOUND_ANALOG_PIN);
  int motion = digitalRead(PIR_PIN);  // 0 or 1
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  int pressureRaw = analogRead(A2);

  
  // ±4g 기준으로 정규화 (원래는 ±2g, 그래서 ×2)
  float accelX = (ax / 16384.0) * 10.0;
  float accelY = (ay / 16384.0) * 10.0;
  float accelZ = (az / 16384.0) * 10.0;

  
  // magnitude 계산
  float accelMag = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
  
  // 0~4 범위로 정규화 (최댓값 6.9 기준)
  float normAccelMag = min((accelMag / 6.9) * 20.0, 20.0);  // 혹시 몰라서 max 제한

  unsigned long seconds = millis() / 1000;
  String timestamp = String(seconds) + "s";
  
  bool valid = true;
  String reason = "";
  if (cds == 0 || cds == 1023) {
    valid = false;
    reason += "\"cds\":\"out_of_range\",";
  }
  if (sound == 0 || sound == 1023) {
    valid = false;
    reason += "\"sound\":\"out_of_range\",";
  }
  if (pressureRaw <= 0 || pressureRaw >= 1023) {  // 예시: 100~1000만 유효 범위
    valid = false;
    reason += "\"pressure\":\"abnormal\",";
  }
  if (normAccelMag < 0 || normAccelMag > 20.0) {
    valid = false;
    reason += "\"accel\":\"invalid_magnitude\",";
  }

  if (!isnan(temp) && !isnan(humid) && valid) {
    String json = "{\"temperature\":" + String(temp, 1) +
                  ",\"humidity\":" + String(humid, 1) +
                  ",\"photoresistor\":" + String(cds) +
                  ",\"sound\":" + String(sound) +
                  ",\"body_detection\":" + String(motion) + 
                  ",\"pressure\":" + String(pressureRaw) +
                  ",\"accelerator\":" + String(normAccelMag, 2) +
                  ",\"timestamp\":\"" + timestamp + "\"}\n";
    bluetooth.println(json);
  } else {
    // [EXCEPTION HANDLING ADDED]
    String errorJson = "{\"error\":\"Sensor read failure or out-of-range\",";
    if (isnan(temp) || isnan(humid)) {
      errorJson += "\"dht\":\"fail\",";
    }
    if (reason.length() > 0) {
      reason.remove(reason.length() - 1);
      errorJson += reason;
    } 
    errorJson += "}";
    bluetooth.println(errorJson);
  }

  delay(2000);
}

void processCommand(String cmd) {
  cmd.trim();

  if (cmd == "VIBRATE:ALL") {
    digitalWrite(VIB1_PIN, HIGH);
    digitalWrite(VIB2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB1_PIN, LOW);
    digitalWrite(VIB2_PIN, LOW);
    bluetooth.println("{\"status\":\"ALL vibrated\"}");
  }
  else if (cmd == "VIBRATE:SPOT1") {
    digitalWrite(VIB1_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB1_PIN, LOW);
    bluetooth.println("{\"status\":\"SPOT1 vibrated\"}");
  }
  else if (cmd == "VIBRATE:SPOT2") {
    digitalWrite(VIB2_PIN, HIGH);
    delay(1000);
    digitalWrite(VIB2_PIN, LOW);
    bluetooth.println("{\"status\":\"SPOT2 vibrated\"}");
  }
  else {
    bluetooth.println("{\"error\":\"Unknown command\"}");
  }
}
