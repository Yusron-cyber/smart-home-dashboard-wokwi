# Smart Home Automation (ESP32 Wokwi + Web Dashboard)

Sistem IoT Smart Home Automation berbasis **ESP32**, disimulasikan menggunakan **Wokwi**, dan terhubung secara realtime dua arah dengan **Web Dashboard** via protokol **MQTT (HiveMQ Public Broker)**.

---

## 📋 Daftar Komponen

| Komponen | Tipe / Model | Pin ESP32 | Keterangan |
|---|---|---|---|
| **Microcontroller** | ESP32 DevKit v4 | - | Otak utama sistem |
| **Sensor Suhu & Kelembaban** | DHT22 | GPIO 15 | Membaca suhu (°C) dan kelembaban udara (%) |
| **Sensor Gerak** | PIR Motion Sensor | GPIO 13 | Deteksi keberadaan orang / maling |
| **Sensor Cahaya** | LDR Photoresistor | GPIO 34 (Analog) | Membaca intensitas cahaya ruangan |
| **Lampu Ruang Tamu** | LED Hijau + Resistor 220Ω | GPIO 2 | Simulasi penerangan ruang tamu |
| **Lampu Kamar Tidur** | LED Kuning + Resistor 220Ω | GPIO 4 | Simulasi penerangan kamar tidur |
| **Kipas / AC** | LED Biru + Resistor 220Ω | GPIO 16 | Simulasi kipas angin / AC |
| **Smart Door Lock** | Micro Servo (SG90) | GPIO 18 | Simulasi kunci pintu otomatis (0° Kunci, 90° Buka) |
| **Buzzer Alarm** | Piezo Buzzer | GPIO 19 | Alarm keamanan suara |
| **Display Status** | LCD 16x2 (I2C) | SDA: GPIO 21, SCL: GPIO 22 | Tampilan status suhu, PIR, dan aktuator lokal |

---

## 🔌 Detail Wiring Rangkaian

1. **Power Supply & Ground:**
   - Semua pin `VCC` sensor (DHT22, PIR, LDR) terhubung ke pin `3V3` ESP32.
   - Pin `VCC` / `V+` Servo dan LCD terhubung ke pin `5V` (VIN) ESP32.
   - Semua pin `GND` komponen terhubung ke pin `GND` ESP32 (GND.1 / GND.2).

2. **Sensor:**
   - **DHT22**: `SDA/OUT` -> GPIO 15
   - **PIR**: `OUT` -> GPIO 13
   - **LDR**: `AO` -> GPIO 34

3. **Aktuator:**
   - **LED Hijau (Ruang Tamu)**: Anoda -> Resistor 220Ω -> GPIO 2 | Katoda -> GND
   - **LED Kuning (Kamar)**: Anoda -> Resistor 220Ω -> GPIO 4 | Katoda -> GND
   - **LED Biru (Kipas/AC)**: Anoda -> Resistor 220Ω -> GPIO 16 | Katoda -> GND
   - **Servo Door**: Signal/PWM -> GPIO 18
   - **Buzzer**: Pin Positif (+) -> GPIO 19 | Pin Negatif (-) -> GND

4. **I2C LCD 16x2:**
   - `SDA` -> GPIO 21
   - `SCL` -> GPIO 22

---

## 🚀 Cara Menjalankan

### 1. Buka Simulasi di Wokwi
1. Masuk ke [https://wokwi.com](https://wokwi.com) dan buat project baru dengan memilih **ESP32**.
2. Salin isi file `sketch.ino` ke tab kode **sketch.ino** di Wokwi.
3. Salin isi file `diagram.json` ke tab **diagram.json** di Wokwi.
4. Di tab **Library Manager** (atau buat file `libraries.txt`), pastikan library berikut terpasang:
   - `PubSubClient`
   - `DHT sensor library`
   - `Adafruit Unified Sensor`
   - `ESP32Servo`
   - `LiquidCrystal I2C`
   - `ArduinoJson`
5. Klik tombol **Play / Start Simulation** (tombol hijau).
6. Perhatikan Serial Monitor: ESP32 akan terhubung ke WiFi `Wokwi-GUEST` dan MQTT HiveMQ broker.
7. atau Masuk ke [https://wokwi.com/projects/480092209017734145](https://wokwi.com/projects/480092209017734145)
### 2. Buka Web Dashboard
1. Double-click file `index.html` untuk membukanya di browser (Chrome, Edge, Firefox, dll).
2. Status badge di pojok kanan atas akan berubah hijau (**Broker: Connected** dan **ESP32: Online**).
3. Sekarang kamu bisa mengontrol saklar, pintu, alarm, dan memantau grafik suhu secara langsung!
4. atau masuk ke [https://smart-home-dashboard-wokwi.vercel.app/](https://smart-home-dashboard-wokwi.vercel.app/)
