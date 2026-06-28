# Bird Alert

Desk-to-desk alert project (ESP32 + MQTT + optional display).

## Hardware and pins

See **[docs/hardware.md](docs/hardware.md)** for MCU, display, GPIO table, and profile header reference.

## Display + touch stack (recommended)

See **[docs/recommended-display-stack.md](docs/recommended-display-stack.md)** for **LovyanGFX** vs **TFT_eSPI** vs Adafruit + **XPT2046_Touchscreen_TT** when you need touch aligned to pixels and calibration.

## WiFi setup

See **[docs/wifi-setup.md](docs/wifi-setup.md)** for on-device network selection, password entry, and NVS credential storage.

## Device UI

See **[docs/device-ui.md](docs/device-ui.md)** for the home screen, alert / acknowledge flow, Settings hub, and optional device nickname.

## MQTT server

See **[docs/mqtt-server.md](docs/mqtt-server.md)** for Mosquitto install and configuration on Raspberry Pi OS (topics, auth, firewall).

## MQTT on ESP32

See **[docs/mqtt-setup.md](docs/mqtt-setup.md)** for broker defaults (`secrets.h`), on-device MQTT wizard, and NVS storage.

## Display bring-up

See **[docs/display-bringup.md](docs/display-bringup.md)** and upload:

`sketches/display_bringup/display_bringup.ino`

**Libraries (Arduino Library Manager):** **LovyanGFX** (lovyan03), **PubSubClient**, **ArduinoJson** (v7).
