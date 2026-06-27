# Recommended display + touch stack (ESP32, ILI9341, XPT2046)

This project started with **Arduino_GFX** (moononournation) for the TFT only. For **touch that lines up with pixels** and **calibration you can save and reload**, the stack below is the most coherent choice on ESP32.

---

## Primary recommendation: **LovyanGFX**

**Library:** [lovyan03/LovyanGFX](https://github.com/lovyan03/LovyanGFX) (Arduino Library Manager: **LovyanGFX**)

**Why**

| Goal | How LovyanGFX helps |
|------|---------------------|
| One place for pins | Subclass `lgfx::LGFX_Device` with `Panel_ILI9341`, `Bus_SPI`, and **`Touch_XPT2046`** in one class (same pattern as upstream examples / `Lovyan_user_setting.ino`). |
| Touch matches graphics | Touch is part of the same device; raw XPT2046 reads are mapped to **display pixel coordinates** with rotation taken into account. |
| Calibration | **`calibrateTouch(uint16_t cal[8], ...)`** produces 8 values; **`setTouchCalibrate(cal)`** applies them. Store `cal` in **NVS / Preferences** and reload on boot so users calibrate once. |
| Performance | Fast SPI path; widely used on ESP32 + ILI9341 + XPT2046 boards (including HSPI wiring like your **cfg1**). |

**Touch chip:** **XPT2046** (SPI, shared bus with TFT, separate **`pin_cs`** for touch — e.g. GPIO **33** on many boards; **confirm on your ELEGOO schematic**).

**Caveat:** You move off Arduino_GFX; you re-specify panel + bus + touch in a small `LGFX` subclass (similar effort to TFT_eSPI `User_Setup.h`, but stays in **your repo** as `.h` files).

---

## Strong alternative: **TFT_eSPI** (+ touch in same config)

**Library:** [Bodmer/TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)

**Why:** Very fast, huge ESP32 + ILI9341 + XPT2046 example set; touch CS / IRQ and optional touch handling are configured in **`User_Setup.h`** next to the TFT pins so hardware stays in one file.

**Trade-offs:** Configuration lives **inside the library tree** unless you use **PlatformIO** `build_flags` / `board_build.embed_txtfiles` or a copied `User_Setup` — slightly more awkward than LovyanGFX “all in project `.h`” for some teams. Calibration is usually **roll your own** or from community snippets (not as unified as LovyanGFX’s `calibrateTouch` API).

---

## If you stay on **Adafruit_GFX** + **Adafruit_ILI9341**

**Touch / calibration helper:** [tedtoal/XPT2046_Touchscreen_TT](https://github.com/tedtoal/XPT2046_Touchscreen_TT) with **`TS_Display`** — documented **touch/release events**, **mapping**, and **calibration** examples.

**Trade-off:** **`TS_Display` expects a display class derived from `Adafruit_GFX`** (README uses `Adafruit_ILI9341`). **Arduino_GFX** is not a drop-in. Graphics are slower on ESP32 than LovyanGFX / TFT_eSPI for the same panel.

Use this path only if you explicitly want the Adafruit ecosystem.

---

## Not recommended for your stated goals

| Stack | Reason |
|-------|--------|
| **Arduino_GFX + Adafruit_TouchScreen** | Adafruit TouchScreen is for **raw 4-wire resistive to ADC**, not **XPT2046 SPI**. |
| **Arduino_GFX + XPT2046 only** | Works for reads, but you **manually** glue rotation, mapping, and calibration; no first-class `calibrateTouch` in GFX. |
| **LVGL** (full UI) | Excellent for complex UIs; **heavy** for a simple bird-alert UI unless you already want LVGL widgets and input device plumbing. |

---

## Suggested migration order (LovyanGFX)

1. Add **LovyanGFX**; create `LGFX_Elegoo28.hpp` (or similar) with your known-good **HSPI** TFT pins and **Touch_XPT2046** (`pin_cs`, optional `pin_int`, same SPI host/pins as TFT per LovyanGFX docs).
2. Replace `display_bringup` drawing with `LGFX` APIs (`fillScreen`, `drawString`, etc. — close to TFT_eSPI style).
3. Run **corner calibration** once; print or save `uint16_t cal[8]`; **`setTouchCalibrate`** on boot.
4. In `loop`, use **`getTouch`** / touch helpers to hit-test your drawn regions (buttons, etc.).

---

## Pin reminder (this repo’s verified TFT cfg1)

See **[hardware.md](hardware.md)** — touch **`T_CS`** (and **`T_IRQ`** if used) must come from **your board’s** pinout; do not assume GPIO 33 without checking the ELEGOO doc.
