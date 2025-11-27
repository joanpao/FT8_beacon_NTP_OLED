# FT8_beacon_NTP_OLED

📡 MULTI_BEACON_FT8
A multiband FT8 Beacon using ESP32 + Si5351 + SSD1306 OLED + Etherlink library
Author: EA5JTT

📝 Description
This project implements a multiband FT8 beacon using:
- ESP32 (LilyGO T3 / T-Beam / compatible boards)
- Si5351 RF signal generator (Etherlink library)
- SSD1306 OLED display (128×64 I²C)
- NTP + WiFi for accurate timing

FT8 transmission
- A carousel system for automatic multiband sequence
 - FT8 encoding using JTEncode
- The device automatically transmits FT8 messages synchronized to standard FT8 timing (every 15 seconds) while cycling through multiple amateur radio bands.

The OLED display provides real-time information such as:
- Current band
- FT8 status
- Transmission frequency
- Initialization steps

✨ Features
✔ FT8 modulation generated with JTEncode
✔ Accurate time sync via NTP
✔ Rock-stable RF with Si5351
✔ Automatic multiband carousel
✔ OLED status display (no flicker)
✔ Modular and easy-to-modify code
✔ Configurable Si5351 correction ppm
✔ Compatible with Arduino IDE & PlatformIO

🔧 Required Hardware
Component	Description
ESP32 board	Tested on LilyGo T-Beam / T3
Si5351 module	CLK0/CLK1/CLK2 outputs
SSD1306 OLED 128×64	I2C address 0x3C
WiFi connection	Required for NTP

Wiring (OLED) by Lilygo T-Beam / T3 vq.6.1
- OLED Pin	ESP32
- SDA	GPIO 21
- SCL	GPIO 22
- VCC	3.3V
- GND	GND

Wiring (Si5351) by Lilygo T-Beam / T3 vq.6.1
- Si5351 Pin	ESP32
- SDA	GPIO 21
- SCL	GPIO 22
- VCC	3.3V
- GND	GND

RF output is generated from CLK1, you can changer



📚 Required Libraries
Install via Arduino Library Manager:
- JTEncode
- Etherkit Si5351
- Adafruit SSD1306
- Adafruit GFX
- WiFi.h (built-in with ESP32)
- NTP / Time

🚀 How to Compile
- Install Arduino IDE 2.x
- Install ESP32 board support
- Select your board (LilyGO T-Beam or generic ESP32)
- Install the required libraries
- Open MULTI_BEACON_FT8.ino

Configure:
- Callsign
- Locator
- FT8 frequencies

Upload the firmware


📄 License
Free for amateur radio and educational use.
More information about dis project in spanish language in: https://ea5jtt.blogspot.com/2025/11/ft8-tx-beacon-arduino-esp32.html
If you modify or distribute, please credit EA5JTT.

