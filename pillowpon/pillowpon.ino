#include <DHT.h>
#include <SoftwareSerial.h>

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

String command = "";  // Bluetooth 명령 수신 버퍼

void setup() {
  bluetooth.begin(9600);
  dht.begin();

  pinMode(VIB1_PIN, OUTPUT);
  pinMode(VIB2_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(VIB1_PIN, LOW);
  digitalWrite(VIB2_PIN, LOW);
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
  double pressure = 512.0;         // Flex sensor
  double accelerator = 1.23;       // MPU6050

  unsigned long seconds = millis() / 1000;
  String timestamp = String(seconds) + "s";

  if (!isnan(temp) && !isnan(humid)) {
    String json = "{\"temperature\":" + String(temp, 1) +
                  ",\"humidity\":" + String(humid, 1) +
                  ",\"photoresistor\":" + String(cds) +
                  ",\"sound\":" + String(sound) +
                  ",\"body_detection\":" + String(motion) + 
                  ",\"pressure\":" + String(pressure, 2) +
                  ",\"accelerator1\":" + String(accelerator, 2) +
                  ",\"accelerator2\":" + String(accelerator, 2) +
                  ",\"timestamp\":\"" + timestamp+"\"}\n";
    bluetooth.println(json);
  } else {
    bluetooth.println("{\"error\":\"DHT read failed\"}");
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
