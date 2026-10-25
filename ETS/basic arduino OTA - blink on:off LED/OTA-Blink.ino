/*
  OTA-Blink - Tugas OTA (Over-The-Air) ESP32
  Upload firmware tanpa kabel USB lewat WiFi (ArduinoOTA).
*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// --- KONFIGURASI WIFI (GANTI DENGAN WIFI ANDA) ---
const char* ssid = "daviansatu 4G";
const char* password = "davi1234";

// --- KONFIGURASI OTA ---
const char* ota_hostname = "esp32-ota";
const char* ota_password = "IoTOTA";

// --- LED BAWAAN ESP32 ---
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const unsigned long BLINK_INTERVAL = 200; // ms (ubah nilai ini lalu upload via OTA untuk demo)

void setup() {
  Serial.begin(115200);
  Serial.println("\nBooting...");

  pinMode(LED_BUILTIN, OUTPUT);

  // --- Koneksi WiFi ---
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // --- Konfigurasi ArduinoOTA ---
  ArduinoOTA.setHostname(ota_hostname);
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(ota_hostname);
  Serial.println("OTA password: IoTOTA");
}

void loop() {
  // --- Inti OTA: proses update nirkabel ---
  ArduinoOTA.handle();

  // --- Blink LED bawaan (GPIO 2) ---
  static unsigned long last_toggle = 0;
  static bool led_state = false;

  if (millis() - last_toggle >= BLINK_INTERVAL) {
    last_toggle = millis();
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
  }
}
