// Thư viện
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Ngưỡng môi trường
#define TEMP_LOW 26
#define TEMP_HIGH 36
#define SOIL_THRESHOLD 35

// Khai báo DHT
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Khai báo độ ẩm đất
const int soilPin = 13;

// Điều khiển motor
const int relayPin = 21;
unsigned long prevMillis;
bool is_running;

void setup() {
  Serial.begin(115200);

  // Khởi tạo DHT
  dht.begin();

  // Khai báo INPUT cho soil
  pinMode(soilPin, INPUT);

  // RELAY
  pinMode(relayPin, OUTPUT);
  prevMillis = 0;
  is_running = false;
  digitalWrite(relayPin, LOW);
}

float read_soil_moisture() {
  int raw = analogRead(soilPin);
  float percent = map(raw, 4095, 0, 0, 100);
  return constrain(percent, 0, 100);
}

bool check_env_condition(){
  float t = dht.readTemperature();
  float percent = read_soil_moisture();

  return (
    (TEMP_LOW <= t && t <= TEMP_HIGH) &&
    (SOIL_THRESHOLD <= percent)
  );
}

void open_motor(int cycle, int duration, bool skip_env){
  unsigned long currMillis = millis();

  if (!is_running && (currMillis - prevMillis) >= cycle){
    if (!skip_env){
      if (!check_env_condition) return;
    }

    digitalWrite(relayPin, HIGH);
    prevMillis = currMillis;
    is_running = true;
  }

  if (is_running && ((currMillis - prevMillis) >= duration)){
    digitalWrite(relayPin, LOW);
    prevMillis = currMillis;
    is_running = false;
  }
}

void print_debug() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float percent = read_soil_moisture();

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Soil Moisture: ");
  Serial.print(percent);
  Serial.println(" %");

  Serial.println("-------------------------");
}

void loop() {
  open_motor(10000, 3000, false);
  print_debug();
  delay(50);
}