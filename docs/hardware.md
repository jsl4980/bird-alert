# Bird Alert — hardware and pin configuration

This document records the **ESP32**, **display**, and **GPIO** layout used in this repo. When wiring or software drifts, treat this file and the kit’s own schematic as the source of truth; update this doc when you change boards or **`elegoo_28_display.h`**.

---

## 1. MCU and programming interface

| Item | Detail |
|------|--------|
| Board (Arduino IDE) | **ESP32 Dev Module** |
| USB–UART | **CH340** (Windows driver from WCH; see project docs / bring-up notes) |
| Flash / upload | USB cable with **data** lines; **Tools → Port →** correct COM port |
| Serial debug | **115200** baud; use board **EN** reset with Serial Monitor open if logs are missed on cold USB plug |
| Core WiFi | Classic ESP32 **2.4 GHz Wi‑Fi only** (no 5 GHz) — relevant for future MQTT |

---

## 2. Display (2.8 inch SPI TFT)

| Item | Detail |
|------|--------|
| Typical controller | **ILI9341** (240×320), SPI |
| Alternate | Some modules use **ST7789**; firmware can try variant 4 in the bring-up sketch |
| Init profile | LovyanGFX **`Panel_ILI9341`** (see [`LGFX_Elegoo28.hpp`](../sketches/display_bringup/LGFX_Elegoo28.hpp)) |
| Graphics library | **LovyanGFX** (lovyan03) |
| SPI buses (ESP32) | **VSPI** default pins: SCK **18**, MOSI **23**, MISO **19**. **HSPI** common routed GPIO: SCK **14**, MOSI **13**, MISO **12** |

### 2.1 Verified working configuration (this project)

Canonical defines live in **[`sketches/display_bringup/elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h)** (“configuration 1”). Summary:

| Signal | GPIO | Notes |
|--------|------|--------|
| TFT **DC** (RS / D-C) | **2** | Data vs command |
| TFT **CS** | **15** | Chip select |
| TFT **RST** | *none in software* (`-1`) | Reset tied to module / EN on PCB |
| TFT **SCK** (SPI clock) | **14** | **HSPI** |
| TFT **MOSI** (SDI) | **13** | **HSPI** |
| TFT **MISO** (SDO) | **12** | **HSPI** |
| TFT **BL** (backlight) | **21** | Active **HIGH** in bring-up sketch |

Software SPI host: **HSPI** (see `TFT_SPI_HOST` / `Arduino_ESP32SPI(..., HSPI)` in sketch).

---

## 3. Alternate pin maps (historical / other boards)

The bring-up sketch used to switch **`BRINGUP_VARIANT`** 0–4 inside a single `.ino`. That logic is **retired**; this repo’s **ELEGOO 2.8" configuration 1** is the only profile in code, in **`elegoo_28_display.h`**. For other hardware, duplicate that header as a new profile (e.g. `other_board_display.h`) or extend this table manually.

| Variant (old sketch) | SPI host | SCK | MOSI | MISO | CS | DC | RST | BL | Display driver (bring-up) |
|---------|----------|-----|------|------|----|----|-----|-----|----------------------------|
| **0** | VSPI | 18 | 23 | 19 | 15 | 2 | 4 | 21 | ILI9341 (type 1 vs 2 was selectable in old sketch) |
| **1** | HSPI | 14 | 13 | 12 | 15 | 2 | −1 | 21 | ILI9341 — **default / verified** |
| **2** | HSPI | 14 | 13 | 12 | 15 | 2 | 4 | 21 | ILI9341 |
| **3** | VSPI | 18 | 23 | 19 | 15 | 2 | −1 | 21 | ILI9341 |
| **4** | HSPI | 14 | 13 | 12 | 15 | 2 | −1 | 21 | **ST7789** (IPS flag `true` in sketch) |

`RST = −1` means the sketch does not drive a dedicated reset GPIO (reset handled on the PCB).

---

Software SPI: TFT **HSPI** (`HSPI_HOST` / `SPI2_HOST`), touch **VSPI** (`VSPI_HOST` / `SPI3_HOST`) — see [`LGFX_Elegoo28.hpp`](../sketches/display_bringup/LGFX_Elegoo28.hpp).

---

## 4. Touch (XPT2046) — cfg1 (verified board class)

On **ESP32-2432S028 / ELEGOO 2.8" integrated** boards, touch uses a **second SPI bus** (not TFT MOSI/SCK/MISO).

| Signal | GPIO | Notes |
|--------|------|--------|
| **T_CLK** | **25** | VSPI / SPI3 |
| **T_DIN** (MOSI) | **32** | |
| **T_DO** (MISO) | **39** | |
| **T_CS** | **33** | |
| **T_IRQ** | **36** | Active low when pressed; used by LovyanGFX |

TFT remains on **HSPI** **14 / 13 / 12** (see §2.1). Implemented in **`LGFX_Elegoo28.hpp`**.

**Alignment:** XPT2046 detection (`touch driver attached: yes`) is not the same as correct screen coordinates. Use LovyanGFX corner calibration and NVS (`elegoo28_touch_cal.h`) — see [display-bringup.md](display-bringup.md) §3.

---

## 5. Power

| Item | Detail |
|------|--------|
| Primary | **5 V** from USB port on the dev board |
| Logic | **3.3 V** I/O on ESP32; display module is usually 3.3 V logic (follow kit if it allows 5 V only on VCC) |

---

## 6. Related docs

- [display-bringup.md](display-bringup.md) — library install, white-screen troubleshooting, Serial Monitor notes  
- Bring-up sketch — [`sketches/display_bringup/display_bringup.ino`](../sketches/display_bringup/display_bringup.ino)  
- Board profile — [`sketches/display_bringup/elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h)  
- LovyanGFX device — [`sketches/display_bringup/LGFX_Elegoo28.hpp`](../sketches/display_bringup/LGFX_Elegoo28.hpp)  
- Touch NVS cal — [`sketches/display_bringup/elegoo28_touch_cal.h`](../sketches/display_bringup/elegoo28_touch_cal.h)

---

## 7. Changelog (manual)

| Date | Change |
|------|--------|
| *(add when you change hardware)* | |

When you add **MQTT**, **button**, or **second ESP32**, append sections here (broker URL, topics, sender/receiver GPIO).
