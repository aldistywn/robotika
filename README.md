# Robotika — Eksperimen ESP32: Kontrol PID & OTA

![ESP32](https://img.shields.io/badge/ESP32-DevKit-red)
![Arduino](https://img.shields.io/badge/Arduino-IDE%202.3.6-00979D)
![Python](https://img.shields.io/badge/Python-3.13-3776AB)
![Blynk](https://img.shields.io/badge/Blynk-IoT-23C48E)
![Jupyter](https://img.shields.io/badge/Jupyter-Notebook-F37626)

Repositori eksperimen mata kuliah Robotika. Tahap **ETS** berisi lima modul
kontrol kecepatan motor DC berbasis PID pada kit iMCLab, mulai dari kendali
*onboard* di ESP32, kendali dari Python, integrasi dashboard IoT Blynk, hingga
pembaruan firmware *Over-The-Air* (OTA) melalui WiFi dan cloud.

---

## Struktur Repo

```text
Robotika/
├── ETS/                              # Eksperimen Tengah Semester
│   ├── basic arduino - PID arduino/          # Modul 1
│   │   └── pid_arduino/pid_arduino.ino
│   ├── python arduino - PID python/          # Modul 2
│   │   ├── pid_python.py
│   │   ├── imclab.py
│   │   └── imclab_arduino_python/
│   ├── blynk arduino - PID blynk/            # Modul 3
│   │   ├── iot-pid-python.ipynb
│   │   ├── imclab.py
│   │   └── imclab_arduino_python/
│   ├── basic arduino OTA - blink on:off LED/ # Modul 4
│   │   └── OTA-Blink.ino
│   ├── arduino OTA - Blynk/                  # Modul 5
│   │   ├── OTA-Blynk.ino
│   │   ├── Settings.h
│   │   └── (header Blynk.Edgent)
│   ├── dokumentasi/                          # Tangkapan layar & foto alat
│   └── laporan/                              # Laporan tiap modul (.md)
└── README.md
```

---

## Eksperimen ETS

| Modul | Topik | Firmware / Script | Laporan |
|:---:|---|---|---|
| 1 | PID *onboard* di ESP32 (tanpa komputer) | [`pid_arduino.ino`](ETS/basic%20arduino%20-%20PID%20arduino/pid_arduino/pid_arduino.ino) | [Buka](ETS/laporan/modul1_pid_arduino.md) |
| 2 | PID di Python + GUI Tkinter/Matplotlib | [`pid_python.py`](ETS/python%20arduino%20-%20PID%20python/pid_python.py) | [Buka](ETS/laporan/modul2_pid_python.md) |
| 3 | PID Python + dashboard IoT Blynk | [`iot-pid-python.ipynb`](ETS/blynk%20arduino%20-%20PID%20blynk/iot-pid-python.ipynb) | [Buka](ETS/laporan/modul3_pid_blynk.md) |
| 4 | OTA firmware via WiFi (ArduinoOTA) | [`OTA-Blink.ino`](ETS/basic%20arduino%20OTA%20-%20blink%20on:off%20LED/OTA-Blink.ino) | [Buka](ETS/laporan/modul4_ota_blink.md) |
| 5 | OTA cloud + provisioning (Blynk.Edgent/Blynk.Air) | [`OTA-Blynk.ino`](ETS/arduino%20OTA%20-%20Blynk/OTA-Blynk.ino) | [Buka](ETS/laporan/modul5_ota_blynk.md) |


---

## Hardware & Pin Mapping

Kit **iMCLab** (Internet-Based Motor Control Lab): ESP32 + driver motor L298N +
motor DC + encoder piringan 2 lubang + LED indikator.

| Fungsi | GPIO | Catatan |
|---|---|---|
| Motor IN1 | 27 | Arah putaran |
| Motor IN2 | 26 | Arah putaran |
| Motor EN (PWM) | 12 | `ledcWrite`, 30 kHz, 8-bit |
| Encoder | 13 | Interrupt `RISING`, 2 lubang/piringan |
| LED onboard | 2 | Indikator / blink demo |

- Modul 1, 2, 3: PID dihitung di luar motor (onboard/Python), PWM channel 1.
- Modul 5: motor memakai LEDC channel 8, LED indikator Edgent channel 1
  (dipisahkan agar tidak konflik).

---

## Software Stack

| Komponen | Versi / Keterangan |
|---|---|
| Arduino IDE | 2.3.6 |
| ESP32 core | 2.0.11 |
| Library Blynk | **1.3.2** (wajib, untuk Blynk.Edgent) |
| Python | 3.13 |
| Library Python | `pyserial`, `numpy`, `matplotlib`, `tkinter`, `BlynkLib` |
| Notebook | Jupyter (VS Code) |
| Toolchain OTA | `ArduinoOTA`, `Blynk.Edgent`, `Blynk.Air` |

---

## Quick Start

### 1. Firmware Arduino

1. Buka file `.ino` di Arduino IDE. Pastikan **nama folder = nama file** `.ino`
   (mis. `OTA-Blynk/OTA-Blynk.ino`).
2. Pilih board:
   - Modul 1 & 4: `DOIT ESP32 DEVKIT V1`
   - Modul 5: `ESP32 Dev Module` dengan partition
     **Minimal SPIFFS (1.9MB APP with OTA)**
3. Set baud Serial Monitor `115200`, lalu **Upload**.
4. Modul 4 & 5: setelah upload pertama via USB, pembaruan berikutnya bisa
   dilakukan via WiFi (ArduinoOTA) atau Blynk.Air.

### 2. PID Python (Modul 2)

```bash
cd "ETS/python arduino - PID python"
python3 -m venv .venv && source .venv/bin/activate
pip install numpy matplotlib pyserial
python pid_python.py
```

Port ESP32 dideteksi otomatis melalui USB VID/PID (`10C4:EA60`, `1A86:55D4`,
`1A86:7523`, `16D0:0613`, `2341:8036`) pada baud 115200.

### 3. Notebook Blynk (Modul 3)

1. Buat template & device di [Blynk.Console](https://blynk.cloud), salin
   **Auth Token**.
2. Isi token pada `iot-pid-python.ipynb`, lalu **Run All**.
3. Kontrol setpoint/Kp/Ki/Kd dari aplikasi Blynk atau dashboard web.

### 4. Membaca Laporan

Buka file `.md` di `ETS/laporan/` lalu `Cmd+Shift+V` (Markdown Preview).
Untuk merender diagram, pasang ekstensi **Markdown Preview Mermaid Support**.


---

## Author

**Ayudya Aldi Setiawan** — [github.com/aldistywn](https://github.com/aldistywn)

Repositori: [github.com/aldistywn/robotika](https://github.com/aldistywn/robotika)
