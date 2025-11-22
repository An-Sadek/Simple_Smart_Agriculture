#define BLYNK_TEMPLATE_ID "TMPL6ButUfLKP"
#define BLYNK_TEMPLATE_NAME "Smart Farm"
#define BLYNK_AUTH_TOKEN "ib5brfoyf2805qn_V7xbJxgCCg1Rp0xQ"

// Thư viện
#include <DHT.h>
#include <Wire.h>
//#include <LiquidCrystal_I2C.h>

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi
char ssid[] = "1111";
char pass[] = "10122003";

// Ngưỡng môi trường
#define TEMP_LOW 26
#define TEMP_HIGH 36
#define SOIL_THRESHOLD 10

// DHT
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil
const int soilPin = 36;

// Relay (motor)
const int relayPin = 12;
unsigned long prevMillis;
bool is_running = false;

BlynkTimer timer;

// ---- ĐỌC ĐỘ ẨM ĐẤT ----
float read_soil_moisture() {
  int raw = analogRead(soilPin);
  float percent = map(raw, 4095, 0, 0, 100);
  return constrain(percent, 0, 100);
}

// ---- KIỂM TRA MÔI TRƯỜNG ----
bool check_env_condition() {
  float t = dht.readTemperature();
  float percent = read_soil_moisture();

  return (
    (TEMP_LOW <= t && t <= TEMP_HIGH) &&
    (percent <= SOIL_THRESHOLD)
  );
}

// ---- HÀM ĐIỀU KHIỂN MOTOR TỰ ĐỘNG ----
void open_motor(int cycle, int duration, bool skip_env) {
  unsigned long currMillis = millis();

  if (!is_running && (currMillis - prevMillis) >= cycle) {
    if (!skip_env) {
      if (!check_env_condition()) return;
    }

    digitalWrite(relayPin, HIGH);
    prevMillis = currMillis;
    is_running = true;
  }

  if (is_running && ((currMillis - prevMillis) >= duration)) {
    digitalWrite(relayPin, LOW);
    prevMillis = currMillis;
    is_running = false;
  }
}

// ---- GỬI DỮ LIỆU LÊN BLYNK ----
void send_to_blynk() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float soil = read_soil_moisture();

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, soil);

  Serial.print("T = ");
  Serial.print(t);
  Serial.print("  H = ");
  Serial.print(h);
  Serial.print("  Soil = ");
  Serial.println(soil);
}

// ---- BLYNK ĐIỀU KHIỂN MOTOR (V3) ----
// Switch ON/OFF
BLYNK_WRITE(V3) {
  int state = param.asInt();
  if (state == 1) {
    digitalWrite(relayPin, HIGH);
    is_running = true;
  } else {
    digitalWrite(relayPin, LOW);
    is_running = false;
  }
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  pinMode(soilPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, LOW);

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Gửi dữ liệu mỗi 1 giây
  timer.setInterval(1000L, send_to_blynk);
}

void loop() {
  Blynk.run();
  timer.run();
  int raw = analogRead(soilPin);
  Serial.println(raw);
  delay(500);

  // Chế độ auto motor
  open_motor(10000, 3000, false);
}
