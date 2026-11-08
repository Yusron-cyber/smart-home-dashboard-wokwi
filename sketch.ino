#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ================= PIN DEFINITIONS =================
#define PIN_DHT          15   // Sensor Suhu & Kelembaban (DHT22)
#define PIN_PIR          13   // Sensor Gerak (PIR)
#define PIN_LDR          34   // Sensor Cahaya (Analog LDR)
#define PIN_LED_LIVING    2   // Lampu Ruang Tamu (Hijau)
#define PIN_LED_BEDROOM   4   // Lampu Kamar Tidur (Kuning)
#define PIN_LED_FAN      16   // Kipas / AC (Biru)
#define PIN_SERVO        18   // Smart Door Lock (Servo)
#define PIN_BUZZER       19   // Alarm Bahaya (Buzzer)

// ================= KOMPONEN OBJECTS =================
#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);
Servo doorServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= WIFI & MQTT CONFIG =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Menggunakan Public Broker HiveMQ (Support TCP 1883 & WebSocket 8000/8884)
const char* mqtt_server = "broker.hivemq.com";
const int   mqtt_port   = 1883;

// Topic MQTT (bisa disesuaikan agar unik)
const char* TOPIC_TELEMETRY = "smarthome/amri/telemetry";
const char* TOPIC_STATUS    = "smarthome/amri/status";
const char* TOPIC_CMD_BASE  = "smarthome/amri/cmd/#";

WiFiClient espClient;
PubSubClient client(espClient);

// ================= STATE PERANGKAT =================
bool stateLivingLight  = false;
bool stateBedroomLight = false;
bool stateFan          = false;
bool stateDoorOpen     = false;
bool stateAlarm        = false;
bool autoMode          = false; // Auto lamp saat gelap & auto fan saat panas

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 2000; // Kirim data tiap 2 detik

// ================= FUNCTION PROTOTYPES =================
void setupWiFi();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendTelemetry();
void updateActuators();
void updateLCD(float temp, float hum, int ldrVal, bool motion);

void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Initializing Smart Home System...");

  // Setup Pin Modes
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_LED_LIVING, OUTPUT);
  pinMode(PIN_LED_BEDROOM, OUTPUT);
  pinMode(PIN_LED_FAN, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Inisialisasi Servo Pintu
  doorServo.setPeriodHertz(50);
  doorServo.attach(PIN_SERVO, 500, 2400);
  doorServo.write(0); // Pintu tertutup (0 derajat)

  // Inisialisasi Sensor DHT & LCD
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Home IoT");
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi");

  // Inisialisasi WiFi & MQTT
  setupWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  delay(1500);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Kirim data sensor berkala
  unsigned long now = millis();
  if (now - lastTelemetryTime >= TELEMETRY_INTERVAL) {
    lastTelemetryTime = now;
    sendTelemetry();
  }

  // Handle Alarm Buzzer beep jika aktif
  if (stateAlarm) {
    tone(PIN_BUZZER, 1000, 100);
    delay(100);
  } else {
    noTone(PIN_BUZZER);
  }
}

// ================= SETUP WIFI =================
void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
}

// ================= RECONNECT MQTT =================
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT Broker...");
    String clientId = "ESP32-SmartHome-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), TOPIC_STATUS, 1, true, "offline")) {
      Serial.println(" Terhubung!");
      client.publish(TOPIC_STATUS, "online", true);
      client.subscribe(TOPIC_CMD_BASE);
      Serial.print("Subscribed ke: ");
      Serial.println(TOPIC_CMD_BASE);
    } else {
      Serial.print(" Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 3 detik...");
      delay(3000);
    }
  }
}

// ================= TERIMA PERINTAH DARI DASHBOARD =================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  Serial.print("📩 Pesan diterima [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  String strTopic = String(topic);

  if (strTopic.endsWith("/living_light")) {
    stateLivingLight = (message == "ON" || message == "1");
  } else if (strTopic.endsWith("/bedroom_light")) {
    stateBedroomLight = (message == "ON" || message == "1");
  } else if (strTopic.endsWith("/fan")) {
    stateFan = (message == "ON" || message == "1");
  } else if (strTopic.endsWith("/door")) {
    stateDoorOpen = (message == "OPEN" || message == "1");
  } else if (strTopic.endsWith("/alarm")) {
    stateAlarm = (message == "ON" || message == "1");
  } else if (strTopic.endsWith("/auto_mode")) {
    autoMode = (message == "ON" || message == "1");
  }

  updateActuators();
  sendTelemetry(); // Update status langsung ke dashboard
}

// ================= UPDATE STATUS FISIK PERANGKAT =================
void updateActuators() {
  digitalWrite(PIN_LED_LIVING, stateLivingLight ? HIGH : LOW);
  digitalWrite(PIN_LED_BEDROOM, stateBedroomLight ? HIGH : LOW);
  digitalWrite(PIN_LED_FAN, stateFan ? HIGH : LOW);
  
  // Servo 0 derajat = Terkunci/Tutup, 90 derajat = Terbuka
  doorServo.write(stateDoorOpen ? 90 : 0);
}

// ================= BACA SENSOR & KIRIM TELEMETRI =================
void sendTelemetry() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();
  if (isnan(temp)) temp = 0.0;
  if (isnan(hum)) hum = 0.0;

  int ldrRaw = analogRead(PIN_LDR);
  // Konversi LDR ke estimasi persentase kecerahan cahaya (0-100%)
  int lightPercent = map(ldrRaw, 4095, 0, 0, 100);
  lightPercent = constrain(lightPercent, 0, 100);

  bool motion = (digitalRead(PIN_PIR) == HIGH);

  // Jika Auto Mode aktif
  if (autoMode) {
    // Jika gelap (< 30%), otomatis hidupkan lampu ruang tamu
    stateLivingLight = (lightPercent < 30);
    // Jika suhu > 28°C, otomatis hidupkan kipas/AC
    stateFan = (temp > 28.0);
    updateActuators();
  }

  // Buat JSON payload
  StaticJsonDocument<300> doc;
  doc["temp"]        = round(temp * 10) / 10.0;
  doc["humidity"]    = round(hum * 10) / 10.0;
  doc["light"]       = lightPercent;
  doc["motion"]      = motion;
  doc["livingLight"] = stateLivingLight;
  doc["bedroomLight"]= stateBedroomLight;
  doc["fan"]         = stateFan;
  doc["door"]        = stateDoorOpen;
  doc["alarm"]       = stateAlarm;
  doc["autoMode"]    = autoMode;

  char jsonBuffer[350];
  serializeJson(doc, jsonBuffer);

  client.publish(TOPIC_TELEMETRY, jsonBuffer);
  Serial.print("📤 Telemetri terkirim: ");
  Serial.println(jsonBuffer);

  updateLCD(temp, hum, lightPercent, motion);
}

// ================= UPDATE DISPLAY LCD =================
void updateLCD(float temp, float hum, int light, bool motion) {
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)temp);
  lcd.print("C H:");
  lcd.print((int)hum);
  lcd.print("% L:");
  lcd.print(light);
  lcd.print("%  ");

  lcd.setCursor(0, 1);
  lcd.print("PIR:");
  lcd.print(motion ? "MOV" : "---");
  lcd.print(" D:");
  lcd.print(stateDoorOpen ? "OPEN" : "LOCK");
  lcd.print(" F:");
  lcd.print(stateFan ? "ON " : "OFF");
}
