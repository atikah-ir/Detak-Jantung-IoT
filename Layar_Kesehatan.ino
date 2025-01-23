#define BLYNK_TEMPLATE_ID "TMPL6LXlpu3RW"
#define BLYNK_TEMPLATE_NAME "Kesehatan"
#define REPORTING_PERIOD_MS 1000  // Interval waktu pelaporan data (1 detik)

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_SSD1306.h>  // Perubahan library OLED
#include <Adafruit_GFX.h>

// Konfigurasi tampilan OLED
#define DS18B20 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Gunakan -1 jika tidak ada pin reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char auth[] = "72cKoVMzqLTley9cUxYr3uKwjF0dKRmK";
char ssid[] = "V2043";
char pass[] = "30september2002";
PulseOximeter pox;

float bodytemperature;
float BPM, SpO2;
uint32_t tsLastReport = 0;

OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);

void onBeatDetected() {
  Serial.println("Detak Terdeteksi!");
}

void setup() {
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  Blynk.begin(auth, ssid, pass);
  Serial.println("Memulai Pulse Oximeter..");

  // Inisialisasi sensor MAX30100
  if (!pox.begin()) {
    Serial.println("GAGAL");
    for (;;);
  } else {
    Serial.println("BERHASIL");
    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Inisialisasi sensor suhu DS18B20
  sensors.begin();

  // Inisialisasi tampilan OLED dengan alamat I2C 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Gagal menginisialisasi tampilan OLED!");
  } else {
    Serial.println("Tampilan OLED berhasil diinisialisasi");
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.println("Kesehatan");
    display.display();
  }
}

void loop() {
  pox.update();
  Blynk.run();
  BPM = pox.getHeartRate();
  SpO2 = pox.getSpO2();
  sensors.requestTemperatures();
  bodytemperature = sensors.getTempCByIndex(0);

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.print("Detak jantung: ");
    Serial.print(BPM);
    Serial.print(" bpm / SpO2: ");
    Serial.print(SpO2);
    Serial.println(" %");
    Serial.print("Suhu Tubuh: ");
    Serial.print(bodytemperature);
    Serial.println("°C");

    // Menampilkan data di OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("DJ: ");
    display.print(BPM);
    display.print(" bpm");
    display.setCursor(0, 25);
    display.print("SpO2: ");
    display.print(SpO2);
    display.print(" %");
    display.setCursor(0, 40);
    display.print("Suhu: ");
    display.print(bodytemperature);
    display.print("°C");
    display.display();

    // Mengirim data ke Blynk
    Blynk.virtualWrite(V5, BPM);
    Blynk.virtualWrite(V6, SpO2);
    Blynk.virtualWrite(V7, bodytemperature);

    tsLastReport = millis();
  }
}
