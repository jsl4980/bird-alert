# Device UI (on-device touch screens)

Bird Alert runs on an **ELEGOO 2.8" ESP32** display (**320×240** landscape, resistive touch). This guide covers the **home screen**, **Settings hub**, and **alert / acknowledge** flow from the user's perspective.

For Mosquitto install and MQTT topics, see **[mqtt-server.md](mqtt-server.md)**. For broker connection on the ESP32 (`secrets.h`, NVS, broker wizard), see **[mqtt-setup.md](mqtt-setup.md)**.

---

## Home screen layout

The home screen has three zones:

| Zone | Content |
|------|---------|
| **Header** | “Bird Alert” title and a connection dot (green = WiFi + MQTT OK, orange = partial, red = offline) |
| **Main area** | Primary action — depends on state (see below) |
| **Footer** | **Settings** (left) and your display name or device ID (right) |

Press firmly on the resistive panel; tap targets are sized for finger use.

---

## Home screen states

### Ready (idle)

- Bird icon and a large red **SPOT A BIRD!** button.
- Tap to publish an alert to all other devices on the network.
- Hint text: “Tap to alert everyone watching”.

If WiFi or MQTT is offline, the button shows **Offline** and the hint says “Open Settings to connect”.

### Alert sent (waiting)

After you tap **SPOT A BIRD!**:

- Button changes to **Alert sent!** with “Waiting for response…” and animated dots.
- You cannot send another alert until someone acknowledges (or the ack timeout clears the state).

### Incoming alert

When another device raises an alert:

- A maroon **BIRD SPOTTED!** banner shows **from** followed by the sender's device ID (e.g. `bird-3a905e`).
- A green **GOT IT** button below the banner.
- Tap **GOT IT** to acknowledge — the sender sees **Acknowledged!** with your device ID.

If several alerts arrive before anyone acks, the **newest** replaces the current one.

### Acknowledged (sender)

When someone acknowledges your alert:

- Full-screen **Acknowledged!** and **by** `{device id}` for about 1.5 seconds.
- Then returns to the ready (idle) screen.

---

## Settings hub

Tap **Settings** in the footer to open:

| Row | Action |
|-----|--------|
| **My name** | Optional friendly name (e.g. “Kitchen window”). Empty = show device ID instead. Stored in NVS on this device only. |
| **Wi-Fi** | Opens the WiFi wizard (network list, password, connect). See **[wifi-setup.md](wifi-setup.md)**. |
| **MQTT broker** | Opens the broker wizard (host, port, user, password). See **[mqtt-setup.md](mqtt-setup.md)**. |
| **Device ID** | Read-only hardware ID (e.g. `bird-3a905e`) — useful when pairing devices. |

Tap **< Back** in the header to return to the home screen.

WiFi and MQTT wizards return to Settings when finished, not directly to home.

---

## Nickname behaviour

- Each device can set **My name** independently in Settings.
- Your nickname appears in the **footer** on your device.
- Alerts and acks over MQTT still use the stable **device ID** in message payloads; other devices see that ID in “from” / “by” lines unless you later add a shared name registry.
- To clear a nickname, open **My name**, delete all characters, and tap **Done**.

---

## Alert protocol (summary)

Devices exchange JSON events on the `bird/events` topic:

- **raise** — someone spotted a bird; peers show the incoming alert UI.
- **ack** — someone tapped **GOT IT**; the active alert clears on all devices that hold that `alert_id`.

Full message schema and broker setup: **[mqtt-server.md](mqtt-server.md)** §4.

---

## First boot flow

1. Touch calibration wizard (if no NVS calibration)
2. WiFi wizard or auto-connect
3. MQTT broker wizard or auto-connect (if MQTT enabled)
4. Home screen (ready)

See **[display-bringup.md](display-bringup.md)** for upload, calibration flags, and Serial debugging.

---

## Related

- [display-bringup.md](display-bringup.md) — firmware upload and touch calibration
- [wifi-setup.md](wifi-setup.md) — WiFi provisioning wizard
- [mqtt-setup.md](mqtt-setup.md) — ESP32 broker connection (`secrets.h`, NVS)
- [mqtt-server.md](mqtt-server.md) — Mosquitto on Raspberry Pi OS
