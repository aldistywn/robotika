# Modul 5 — OTA Blynk (Blynk.Edgent + Blynk.Air)

## Tujuan

Mengintegrasikan OTA firmware ESP32 dengan platform Blynk secara penuh menggunakan **Blynk.Edgent** (framework provisioning WiFi otomatis) dan **Blynk.Air** (layanan OTA berbasis cloud Blynk). Berbeda dengan Modul 3 (ArduinoOTA via LAN), pada modul ini firmware dapat diperbarui dari mana saja melalui internet. Sekaligus ditambahkan kontrol motor DC via virtual pin Blynk.

---

## Alat & Bahan

| Komponen | Keterangan |
|---|---|
| Kit iMCLab | ESP32 + L298N + encoder (GPIO 27/26/12/13), LED GPIO 2 |
| Jaringan WiFi 2.4 GHz | Untuk ESP32 setelah provisioning |
| Arduino IDE 2.3.6 | Board: ESP32 Dev Module, Partition: Minimal SPIFFS |
| Blynk library v1.3.2 | Versi dengan Blynk.Edgent self-contained (9 header) |
| `OTA-Blynk.ino` + 9 header | Firmware utama + Edgent support files |
| Blynk.Console (blynk.cloud) | Template: "Quickstart Template" (TMPL60XTXooZG) |
| Browser / Blynk App | Provisioning via `http://192.168.4.1` |

---

## Cara Kerja

### Diagram State Machine Edgent (Provisioning + OTA)

```mermaid
flowchart TD
    START([Boot]) --> CHECK{Config\nValid?}

    CHECK -- Tidak --> WAIT["MODE_WAIT_CONFIG\n🔵 LED biru kedip lambat\nESP32 buat WiFi AP\nBlynk Quickstart Template-4RYK"]
    WAIT --> CONFIG["MODE_CONFIGURING\n🔵 LED biru kedip cepat\nUser buka 192.168.4.1\nIsi SSID + Token + Host"]
    CONFIG --> STA["MODE_SWITCH_TO_STA\nGanti mode AP → STA"]

    CHECK -- Ya --> NET
    STA --> NET["MODE_CONNECTING_NET\n🟢 Konek WiFi 2.4GHz"]
    NET --> CLOUD["MODE_CONNECTING_CLOUD\n🟢 Konek blynk.cloud:443"]
    CLOUD --> RUN["MODE_RUNNING\n🟢 LED hijau bernafas\nBlynk Online\nKontrol Motor V0/V1/V2"]

    RUN -->|"Blynk.Air\nNew Shipment"| OTA["MODE_OTA_UPGRADE\n🟣 LED magenta\nDownload .bin via HTTPS\nFlash firmware baru"]
    OTA --> START

    WAIT -->|"Tahan BOOT\n10 detik"| RESET["MODE_RESET_CONFIG\nHapus config dari flash"]
    RESET --> WAIT
```

### Diagram Kontrol Motor via Blynk

```mermaid
flowchart LR
    subgraph APP["📱 Blynk App / Console"]
        V0["V0 Slider\nKecepatan 0–100%"]
        V1["V1 Gauge\nRPM Aktual"]
        V2["V2 Button\nStop Motor"]
    end

    subgraph ESP32["⚙️ ESP32 — OTA-Blynk.ino"]
        BW0["BLYNK_WRITE(V0)\nledcWrite ch8"]
        BW2["BLYNK_WRITE(V2)\nledcWrite 0"]
        RPM_T["BlynkTimer 1s\nreadRPM()"]
    end

    subgraph HW["🔧 Hardware"]
        L298N["L298N\nGPIO 12 EN ch8"]
        ENC["Encoder\nGPIO 13 ISR"]
        MOTOR([Motor DC])
    end

    V0 --> BW0 --> L298N --> MOTOR
    V2 --> BW2 --> L298N
    MOTOR --> ENC --> RPM_T --> V1
```

### Penjelasan Teknis

| Aspek | Detail |
|---|---|
| Provisioning | Blynk.Inject via WiFi AP (bukan BLE), web form 192.168.4.1 |
| Config storage | NVS (`Preferences "blynk"`) — persisten antar reboot & flash |
| LED indikator | LEDC channel 1 (high-speed timer) |
| Motor PWM | LEDC channel 8 (low-speed timer) — tidak konflik |
| Reset config | Tahan BOOT (GPIO 0) ≥ 10 detik → hapus NVS |
| OTA trigger | `BLYNK_WRITE(InternalPinOTA)` → download .bin dari URL Blynk.Air |

### Kode Kunci

```cpp
// Motor: PWM channel 8 (low-speed), terpisah dari LED indikator ch 1
ledcSetup(8, 30000, 8);
ledcAttachPin(enable1Pin, 8);  // GPIO 12

BLYNK_WRITE(V0) {   // slider kecepatan dari Blynk
    int duty = map(constrain(param.asInt(), 0, 100), 0, 100, 0, 255);
    ledcWrite(8, duty);
}

BLYNK_WRITE(V2) {   // tombol stop
    if (param.asInt()) ledcWrite(8, 0);
}

// Kirim RPM ke gauge Blynk (V1) tiap 1 detik
motorTimer.setInterval(1000L, readRPM);

void loop() {
    BlynkEdgent.run();   // state machine: provisioning + cloud + OTA
    motorTimer.run();
}
```

### Alur Update Firmware via Blynk.Air

```mermaid
sequenceDiagram
    participant IDE as Arduino IDE
    participant ESP as ESP32
    participant CLOUD as Blynk.Cloud
    participant DEV as Developer

    DEV->>IDE: Naikkan BLYNK_FIRMWARE_VERSION
    IDE->>IDE: Sketch → Export Compiled Binary (.bin)
    DEV->>CLOUD: Upload .bin ke Blynk.Air → New Shipment
    CLOUD->>ESP: Kirim sinyal OTA (InternalPinOTA + URL)
    ESP->>CLOUD: Download firmware .bin via HTTPS
    ESP->>ESP: Verifikasi MD5 → Flash → Reboot
    ESP->>CLOUD: Reconnect dengan versi firmware baru
```

---

## Hasil Eksperimen

### Reset Config & AP Provisioning Muncul

Serial Monitor menampilkan alur reset konfigurasi: setelah tombol BOOT ditahan 10 detik, config dihapus (`Resetting configuration!`), device masuk `WAIT_CONFIG`, dan AP `blynk.setup` aktif. Di panel WiFi macOS, AP **"Blynk Quickstart Template-4RYK"** terlihat dalam daftar jaringan.

![Arduino IDE + WiFi menu — AP Blynk Quickstart Template-4RYK muncul, Serial Monitor RESET_CONFIG → WAIT_CONFIG](../dokumentasi/Screenshot%202026-10-19%20at%2017.46.41.png)

### Form Provisioning di Browser (192.168.4.1)

Setelah laptop terhubung ke AP, browser membuka `http://192.168.4.1` dan menampilkan form konfigurasi: WiFi SSID, Password, Auth Token (32 karakter), Host (`blynk.cloud`), dan Port (`443`).

![Browser 192.168.4.1 — form provisioning WiFi dan Auth Token](../dokumentasi/Screenshot%202026-10-19%20at%2017.53.06.png)

### Respons Berhasil — "Trying to connect..."

Setelah form disubmit, device merespons dengan JSON `{"status":"ok","msg":"Trying to connect..."}`, menandakan konfigurasi diterima dan disimpan ke flash NVS. ESP32 langsung beralih ke mode STA.

![Browser — respons JSON status ok Trying to connect](../dokumentasi/Screenshot%202026-10-19%20at%2017.58.52.png)

### Device Online di Blynk.Console

Device "PID IoT" muncul **Online** di Blynk.Console. Dashboard menampilkan Setpoint=1000, Kp=0.4, Ki=0.8, Kd=1.2. Indikator **Messages used: 1.3k of 100k** membuktikan komunikasi data antara ESP32 dan Blynk.Cloud sudah berlangsung.

![Blynk.Console — PID IoT Online setelah provisioning Kp=0.4 Ki=0.8 Kd=1.2 Messages 1.3k](../dokumentasi/Screenshot%202026-10-19%20at%2017.59.43.png)

---

## Kesimpulan

Blynk.Edgent + Blynk.Air memberikan solusi OTA yang lebih lengkap dibanding ArduinoOTA (Modul 3): provisioning WiFi tanpa hardcode kredensial, OTA via internet (bukan hanya LAN lokal), dan integrasi dashboard IoT untuk monitoring dan kontrol motor. Pemisahan LEDC channel motor (ch 8) dari LED indikator Edgent (ch 1) memastikan keduanya bekerja tanpa konflik. Keterbatasannya: ukuran binary lebih besar (perlu partition scheme "Minimal SPIFFS") dan memerlukan akun Blynk serta koneksi internet aktif.
