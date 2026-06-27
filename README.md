# Bird Alert

Desk-to-desk alert project (ESP32 + MQTT + optional display).

## Hardware and pins

See **[docs/hardware.md](docs/hardware.md)** for MCU, display, GPIO table, and profile header reference.

## Display + touch stack (recommended)

See **[docs/recommended-display-stack.md](docs/recommended-display-stack.md)** for **LovyanGFX** vs **TFT_eSPI** vs Adafruit + **XPT2046_Touchscreen_TT** when you need touch aligned to pixels and calibration.

## Display bring-up

See **[docs/display-bringup.md](docs/display-bringup.md)** and upload:

`sketches/display_bringup/display_bringup.ino`

**Library:** **LovyanGFX** (lovyan03) via Arduino Library Manager.
