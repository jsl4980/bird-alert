# Display bring-up (ESP32 + 2.8" SPI TFT + touch)

Pin tables for **ELEGOO configuration 1** live in **[hardware.md](hardware.md)**, **[`elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h)**, and **[`LGFX_Elegoo28.hpp`](../sketches/display_bringup/LGFX_Elegoo28.hpp)**.

## 1. Install LovyanGFX (Arduino IDE 2)

1. **Sketch → Include Library → Manage Libraries…**
2. Search **LovyanGFX**
3. Install **LovyanGFX** by **lovyan03**

No `User_Setup.h` — board config is in this repo’s headers.

## 2. Open and upload

1. **File → Open…** → `sketches/display_bringup/display_bringup.ino`
2. **Tools → Board → ESP32 Dev Module**
3. **Tools → Port →** CH340 COM port
4. Upload

### Expected behavior

**First boot (no NVS calibration):**

1. Full-screen message: “Tap each corner target”
2. LovyanGFX draws four corner targets — tap each in order
3. Calibration is saved to NVS and printed on Serial as `cal[8]`
4. **Corner validation** screen (once, right after the wizard) — tap each red target (TL, TR, BR, BL, center)
5. Serial logs `corner TL error dx=… dy=…` for each; green `+` should land on the target
6. Bring-up UI: “Bird Alert”, cyan border, “Tap to test touch”

**Later boots:** NVS cal loads silently — straight to the bring-up UI (no wizard, no corner test) unless NVS was cleared or you set `FORCE_CALIBRATE`.

**WiFi (when `BIRD_ALERT_WIFI_ENABLED` is 1):** After touch setup, the device auto-connects using saved credentials or opens the on-device WiFi wizard. See **[wifi-setup.md](wifi-setup.md)**.

Open Serial Monitor @ **115200**, press **EN** if you miss boot lines.

**Note:** `touch driver attached: yes` only means the XPT2046 driver is configured — alignment requires calibration. `touch: yes` in the main loop means a mapped coordinate, not necessarily a correct one.

## 3. Touch and display alignment

Touch alignment uses **LovyanGFX `calibrateTouch()`** plus **NVS storage** (`elegoo28_touch_cal.h`). Do **not** tune `offset_rotation` or invert Y ranges manually while using stored `cal[8]` — those hacks fight the library’s affine transform and cause diagonal opposition (e.g. physical bottom-left maps to screen top-right).

| Flag in `elegoo_28_display.h` | Purpose |
|-------------------------------|---------|
| `ELEGOO28_DISPLAY_ROTATION` | TFT `setRotation()` — **re-calibrate** if you change this |
| `ELEGOO28_TOUCH_REQUIRE_CALIBRATION` | Run wizard when NVS has no cal for current rotation (default **1**) |
| `ELEGOO28_TOUCH_FORCE_CALIBRATE` | Set to **1**, upload once, then back to **0** to re-calibrate |
| `ELEGOO28_TOUCH_RUN_CORNER_TEST` | Corner validation after a **fresh** wizard only (default **1**; skipped when NVS loads) |
| `ELEGOO28_TOUCH_DEBUG_RAW` | Print raw ADC on Serial (default **0**) |
| `BIRD_ALERT_WIFI_ENABLED` | On-device WiFi setup + auto-connect (default **1**); set **0** for display-only debugging |

### Re-calibrate

1. Set **`ELEGOO28_TOUCH_FORCE_CALIBRATE`** to **`1`** in [`elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h)
2. Upload and complete the corner wizard
3. Set **`ELEGOO28_TOUCH_FORCE_CALIBRATE`** back to **`0`** and upload again (optional — flag only matters on boot)

Or call `elegoo28_touch_cal_clear()` from your own code and reboot.

NVS survives **power cycle**; it is cleared on **re-flash** (acceptable — run the wizard again).

### Success criteria

After calibration and reboot:

- Tap **top-left** → `touch:` near `(0–30, 0–30)` and green `+` on the target
- Tap **bottom-right** → `touch:` near `(290–319, 210–239)` on a 320×240 landscape panel
- No negative coordinates at corners
- Corner validation Serial lines show `OK` (within ±15 px by default)

### If calibration completes but validation fails

| Check | Action |
|-------|--------|
| Wrong diagonal / mirrored | Try `ELEGOO28_DISPLAY_ROTATION` **0, 2, or 3**; clear NVS and re-calibrate for each |
| Display upside-down vs touch | Adjust **panel** `offset_rotation` in `LGFX_Elegoo28.hpp` only (not touch hacks) |
| Wizard won’t start | Confirm touch on **VSPI 25/32/39**; see section 5 |
| Community baseline | As last resort, seed NVS with published 2432S028 `cal[8]` from [radio3-network notes](https://github.com/radio3-network/kit-ESP32-2432S028R/blob/main/notes-ESP32-2432S028R.txt) if interactive cal fails |

## 4. Touch not detected

| Symptom | Action |
|---------|--------|
| No response when pressing | Touch is on **VSPI 25/32/39**, not TFT **14/13/12**. Confirm latest `LGFX_Elegoo28.hpp`. |
| `irq=1` always | No pen-down on IRQ 36 — check touch flex / panel |
| Wizard runs but no advance | Press firmly on each corner target; watch Serial for `raw:` with `DEBUG_RAW=1` |
| `touch:` but wrong position | Re-calibrate (`FORCE_CALIBRATE=1`) — do not revert to manual axis flags |

## 5. White screen (wrong pins / driver)

TFT uses **HSPI** (**14 / 13 / 12**), not VSPI **18 / 23 / 19**. See **`elegoo_28_display.h`**.

## 6. Related

- [recommended-display-stack.md](recommended-display-stack.md) — why LovyanGFX
- [hardware.md](hardware.md) — full GPIO table
