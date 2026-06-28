# WiFi setup (on-device touch UI)

Bird Alert stores WiFi credentials in **ESP32 NVS** (non-volatile storage) via Arduino `Preferences`, in the same `bird_alert` namespace used for touch calibration. Credentials survive reboot and power cycle; they are cleared on **re-flash**.

## First boot

After touch calibration (if needed), the device:

1. Checks NVS for saved SSID and password
2. If found, attempts auto-connect (15 s timeout)
3. If not found or connect fails, opens the **WiFi setup wizard**:
   - Scan nearby networks
   - Tap a network from the scrollable list (tap right/left edge to scroll when many networks appear)
   - Enter password on the on-screen keyboard (open networks connect without a password)
   - On success, credentials are saved to NVS

The home screen shows SSID, IP, and RSSI when connected.

## Change network later

Tap the **WiFi** button in the bottom-right corner of the home screen to re-open the setup wizard. New credentials replace the previous ones in NVS.

## NVS keys

| Key | Content |
|-----|---------|
| `wifi_ssid` | Network name (max 32 chars) |
| `wifi_pass` | Password (max 64 chars, plaintext) |

Namespace: `bird_alert` (shared with touch calibration keys — no conflict).

To clear credentials from code, call `bird_alert_wifi_clear()` in [`bird_alert_wifi_prefs.h`](../sketches/display_bringup/bird_alert_wifi_prefs.h).

## Serial debugging

Open Serial Monitor @ **115200**. Boot and WiFi events are logged:

- `wifi: scanning...`
- `wifi: scan found N networks`
- `wifi: connecting to "..."`
- `wifi: connected, IP=...`
- `wifi: saved credentials for "..."`

## Disable WiFi during display debugging

Set **`BIRD_ALERT_WIFI_ENABLED`** to **`0`** in [`elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h) to skip WiFi setup and restore the original touch crosshair demo.

## Related

- [mqtt-setup.md](mqtt-setup.md) — on-device MQTT broker wizard
- [mqtt-server.md](mqtt-server.md) — Mosquitto broker on Raspberry Pi OS
- [display-bringup.md](display-bringup.md) — upload and touch calibration
- [hardware.md](hardware.md) — ESP32 2.4 GHz Wi-Fi only
