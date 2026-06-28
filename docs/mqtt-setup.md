# MQTT on ESP32 (broker connection)

Bird Alert ESP32 devices connect to your Mosquitto broker after WiFi is up. Broker settings are loaded in this order:

1. **NVS** (if saved from a previous wizard session)
2. **`secrets.h`** defaults (gitignored; copy from [`secrets.h.example`](../sketches/display_bringup/secrets.h.example))

If no host is configured, the **MQTT broker wizard** opens automatically on boot.

## First boot (after WiFi)

1. Device connects to WiFi (auto or WiFi wizard)
2. Loads MQTT config from NVS or `secrets.h`
3. If host is empty or connect fails → **MQTT Broker** wizard:
   - Tap **Host**, **Port**, **User**, or **Password** to edit (on-screen keyboard)
   - Tap **Save & Connect**
4. On success, settings are saved to NVS and the device connects to the broker

You can re-open the broker wizard from **Settings → MQTT broker** on the home screen. See **[device-ui.md](device-ui.md)** for the full on-device UI.

## NVS keys

| Key | Content |
|-----|---------|
| `mqtt_host` | Broker hostname or IP (max 64 chars) |
| `mqtt_port` | TCP port (default 1883) |
| `mqtt_user` | MQTT username |
| `mqtt_pass` | MQTT password (plaintext) |

Namespace: `bird_alert` (shared with WiFi, nickname, and touch calibration keys).

To clear MQTT settings from code, call `bird_alert_mqtt_clear()` in [`bird_alert_mqtt_prefs.h`](../sketches/display_bringup/bird_alert_mqtt_prefs.h).

## secrets.h

Copy the example file and edit for your LAN:

```bash
cp sketches/display_bringup/secrets.h.example sketches/display_bringup/secrets.h
```

Edit host, port, user, and password before the first flash. The on-device wizard can override these later without re-flashing.

**Do not commit `secrets.h`** — it is listed in `.gitignore`.

## Arduino libraries

Install via Library Manager before upload:

- **PubSubClient** (Nick O'Leary)
- **ArduinoJson** (Benoit Blanchon) — v7.x

Existing dependency: **LovyanGFX**.

## Serial debugging

Open Serial Monitor @ **115200**. Look for:

- `device: id=bird-… client_id=bird-…`
- `mqtt: connecting to …`
- `mqtt: connected`
- `mqtt: subscribed to bird/events`
- `mqtt: published raise …` / `mqtt: published ack …`

## Disable MQTT during debugging

Set **`BIRD_ALERT_MQTT_ENABLED`** to **`0`** in [`elegoo_28_display.h`](../sketches/display_bringup/elegoo_28_display.h). WiFi and the home screen still work; alert/MQTT features are skipped.

## Related

- [mqtt-server.md](mqtt-server.md) — Mosquitto install on Raspberry Pi OS (topics, auth, firewall)
- [device-ui.md](device-ui.md) — home screen, alerts, Settings hub
- [wifi-setup.md](wifi-setup.md) — WiFi provisioning
- [display-bringup.md](display-bringup.md) — upload and touch calibration
