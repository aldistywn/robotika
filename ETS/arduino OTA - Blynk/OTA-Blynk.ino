/*************************************************************
  OTA-Blynk - Kontrol Motor DC + OTA berbasis Blynk (Blynk.Air)
  Berbasis contoh Blynk.Edgent > Edgent_ESP32 (Blynk library v1.3.2)

  Fitur:
  - Blynk.Inject : provisioning WiFi lewat aplikasi Blynk (sekali saja)
  - Blynk.Air    : update firmware via WiFi dari Blynk.Console
  - Kontrol motor DC (L298N) + baca RPM encoder

  Pin (sama seperti iMCLab):
    GPIO 27 -> L298N IN1
    GPIO 26 -> L298N IN2
    GPIO 12 -> L298N Enable A (PWM)
    GPIO 13 -> Encoder (interrupt)
    GPIO 2  -> LED indikator status (dipakai Edgent)

  Widget Blynk:
    V0 -> Slider : kecepatan motor (0-100%)
    V1 -> Gauge  : RPM aktual
    V2 -> Button : Stop motor
 *************************************************************/

/* 1. ISI DARI BLYNK CONSOLE (Template > General Settings) */
#define BLYNK_TEMPLATE_ID           "TMPL60XTXooZG"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"

/* 2. Versi firmware - NAIKKAN tiap kali mau push OTA (mis. 0.1.1) */
#define BLYNK_FIRMWARE_VERSION      "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// 3. Pilih board (ESP32 Dev Module)
#define USE_ESP32_DEV_MODULE

#include "BlynkEdgent.h"

// ---------------- PIN MOTOR (sama seperti iMCLab) ----------------
const int motor1Pin1 = 27;      // L298N IN1
const int motor1Pin2 = 26;      // L298N IN2
const int enable1Pin = 12;      // L298N Enable A (PWM)
const byte pin_rpm   = 13;      // Encoder

// PWM motor: channel 8 = low-speed, agar tidak bentrok dengan LED indikator
// (LED indikator Edgent memakai BOARD_LEDC_CHANNEL_1 = 1)
const int freq       = 30000;
const int pwmChannel = 8;
const int resolution = 8;       // 8-bit -> 0..255

// ---------------- VARIABEL RPM ----------------
volatile unsigned long rev = 0;
unsigned long last_rev_count = 0;
unsigned long last_rpm_time  = 0;
float rpm = 0;

void IRAM_ATTR isr_encoder() {
  rev++;
}

BlynkTimer motorTimer;

// ---------------- BACA RPM ----------------
void readRPM() {
  unsigned long current_time = millis();
  unsigned long time_elapsed = current_time - last_rpm_time;

  if (time_elapsed >= 1000) {
    noInterrupts();
    unsigned long current_rev = rev;
    interrupts();

    int holes = 2;  // sesuaikan jumlah lubang piringan encoder
    float rotations = (float)(current_rev - last_rev_count) / holes;
    rpm = rotations / (time_elapsed / 60000.0);

    last_rev_count = current_rev;
    last_rpm_time  = current_time;

    Blynk.virtualWrite(V1, rpm);
  }
}

// ---------------- SLIDER KECEPATAN (V0) ----------------
BLYNK_WRITE(V0) {
  int speed = param.asInt();              // 0 - 100 (%)
  speed = constrain(speed, 0, 100);
  int duty = map(speed, 0, 100, 0, 255);
  ledcWrite(pwmChannel, duty);
}

// ---------------- TOMBOL STOP (V2) ----------------
BLYNK_WRITE(V2) {
  if (param.asInt()) {
    ledcWrite(pwmChannel, 0);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // --- Motor ---
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  digitalWrite(motor1Pin1, HIGH);   // arah putar maju
  digitalWrite(motor1Pin2, LOW);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcWrite(pwmChannel, 0);          // motor mati saat boot

  // --- Encoder ---
  pinMode(pin_rpm, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_rpm), isr_encoder, RISING);

  // --- Blynk Edgent (WiFi provisioning + OTA + cloud) ---
  BlynkEdgent.begin();

  // Baca RPM tiap 1 detik
  motorTimer.setInterval(1000L, readRPM);
}

void loop() {
  BlynkEdgent.run();
  motorTimer.run();
}
