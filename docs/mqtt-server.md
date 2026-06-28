# MQTT server (Raspberry Pi OS)

Bird Alert uses **Eclipse Mosquitto** as the MQTT broker on a Raspberry Pi running **Raspberry Pi OS** (Debian-based) on your home LAN. Mosquitto is lightweight, well supported by ESP32 Arduino libraries, and more than enough for a handful of devices sending occasional alerts.

ESP32 devices connect over **2.4 GHz Wi‑Fi** (see [hardware.md](hardware.md)). The broker should have a **stable LAN IP** so firmware can use a fixed address.

The same `apt` steps work on other Debian/Ubuntu hosts if you are not on Pi OS.

---

## 1. Install Mosquitto

On the Pi (SSH or local terminal):

```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto
```

Check that the service is running:

```bash
systemctl status mosquitto
```

Default listener: **TCP 1883** on all interfaces.

---

## 2. Reserve a static IP for the broker

Give the Pi a fixed address on your router (DHCP reservation) or configure a static IP in Ubuntu netplan. Example broker address used below: **`192.168.1.50`** — replace with your Pi’s actual IP.

Devices and test clients must use this address, not a hostname, unless you run local DNS and prefer names.

---

## 3. Quick smoke test (no auth)

Before locking down access, confirm Mosquitto works on the Pi:

```bash
# Terminal 1 — subscribe
mosquitto_sub -h localhost -t "bird/events" -v

# Terminal 2 — publish
mosquitto_pub -h localhost -t "bird/events" -m '{"v":1,"type":"raise","alert_id":"test-1","from":"test","ts":1}'
```

From another machine on the LAN (replace the IP):

```bash
mosquitto_sub -h 192.168.1.50 -t "bird/events" -v
mosquitto_pub -h 192.168.1.50 -t "bird/events" -m '{"v":1,"type":"raise","alert_id":"test-2","from":"laptop","ts":2}'
```

If the subscribe terminal prints the message, the broker is reachable on the network.

---

## 4. Topic layout

All ESP32 devices publish and subscribe on a **single broadcast channel** for alert lifecycle events. Device identity is carried in the JSON payload (`from`, `alert_id`), not in the topic path.

| Topic | QoS | Direction | Purpose |
|-------|-----|-----------|---------|
| `bird/events` | 0 | all ↔ broker | Alert `raise` and `ack` events (JSON) |
| `bird/status/{device_id}` | 0 | ESP32 → broker | Optional heartbeat / online state (not used yet) |

`{device_id}` is derived from the ESP32 eFuse factory MAC (e.g. `bird-3a905e`). The broker does not assign ids.

### Message format (version 1)

**Raise** (ALERT button on originating device):

```json
{"v":1,"type":"raise","alert_id":"bird-3a905e-48291023","from":"bird-3a905e","ts":48291023}
```

**Ack** (tap alert banner on any device):

```json
{"v":1,"type":"ack","alert_id":"bird-3a905e-48291023","from":"bird-k1f2a3","ts":48295000}
```

| Field | Rule |
|-------|------|
| `v` | Schema version (`1`) |
| `type` | `raise` or `ack` |
| `alert_id` | `{from}-{millis()}` at raise time; unchanged in ack |
| `from` | Device that sent this message |
| `ts` | Sender `millis()` at publish (newest-wins ordering) |

### Watch traffic

```bash
mosquitto_sub -h 192.168.1.50 -u bird_alert -P 'YOUR_PASSWORD' -t 'bird/events' -v
```

Manual test publish:

```bash
mosquitto_pub -h 192.168.1.50 -u bird_alert -P 'YOUR_PASSWORD' -t 'bird/events' \
  -m '{"v":1,"type":"raise","alert_id":"test-1","from":"test","ts":1}'
```

---

## 5. Enable username and password

Do not leave an open broker on a home network long term. Mosquitto uses a password file and disables anonymous clients.

### 5.1 Create a MQTT user

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd bird_alert
```

Enter a strong password when prompted. The `-c` flag creates the file; omit `-c` when adding more users later.

```bash
sudo chown root:mosquitto /etc/mosquitto/passwd
sudo chmod 640 /etc/mosquitto/passwd
```

### 5.2 Broker configuration

Create a drop-in config (keeps the default `mosquitto.conf` intact):

```bash
sudo nano /etc/mosquitto/conf.d/bird-alert.conf
```

```conf
# Bird Alert — LAN broker
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd

# Optional: log connects/disconnects while debugging
# log_type error
# log_type warning
# log_type notice
# connection_messages true
```

Restart and verify:

```bash
sudo systemctl restart mosquitto
systemctl status mosquitto
```

### 5.3 Test with credentials

```bash
mosquitto_sub -h 192.168.1.50 -u bird_alert -P 'YOUR_PASSWORD' -t "bird/events" -v
mosquitto_pub -h 192.168.1.50 -u bird_alert -P 'YOUR_PASSWORD' -t "bird/events" \
  -m '{"v":1,"type":"raise","alert_id":"auth-test","from":"test","ts":3}'
```

Anonymous publish/subscribe should now fail.

---

## 6. Firewall (Raspberry Pi OS)

**Raspberry Pi OS does not install `ufw` by default**, and most Pis ship with **no host firewall enabled**. That is normal: incoming traffic is still blocked from the public internet by your router’s NAT; only devices on your LAN can reach port 1883.

For Bird Alert on a home network, **MQTT username/password (section 5) is the important control**. A host firewall is optional hardening, not a prerequisite.

### 6.1 Check what your Pi is using

Run on the Pi:

```bash
# ufw — usually "not installed" on stock Pi OS
command -v ufw && sudo ufw status verbose || echo "ufw: not installed"

# nftables service — often "inactive" until you configure it
systemctl is-active nftables 2>/dev/null || echo "nftables service: inactive or not enabled"

# Any packet-filter rules loaded?
sudo nft list ruleset

# Mosquitto listening (after install)
sudo ss -tlnp | grep 1883
```

| What you see | Meaning |
|--------------|---------|
| `ufw: not installed` | Default on Pi OS — no action required for MQTT to work |
| `nft list ruleset` is empty and `nftables` inactive | No host firewall; LAN clients can reach 1883 if auth passes |
| `Status: active` from `ufw status` | ufw is managing the firewall — use section 6.2 |
| Non-empty `nft list ruleset` | Custom nftables rules — adjust those instead of installing ufw |

### 6.2 Option A — Do nothing (typical for home LAN)

If the checks above show no firewall, you can skip this section. Ensure:

- Mosquitto has **`allow_anonymous false`** and a password file (section 5).
- The Pi is not port-forwarded on the router (no inbound 1883 from the internet).

### 6.3 Option B — Install ufw (simplest if you want a firewall)

The [official Raspberry Pi securing guide](https://www.raspberrypi.com/documentation/computers/configuration.html#securing-your-raspberry-pi) recommends **ufw** when you want an easy host firewall. It is not preinstalled:

```bash
sudo apt update
sudo apt install ufw
```

**Allow SSH before enabling**, or you can lock yourself out of remote sessions:

```bash
# If you use SSH (adjust port if not 22)
sudo ufw allow OpenSSH

# MQTT from your LAN only (adjust subnet)
sudo ufw allow from 192.168.1.0/24 to any port 1883 proto tcp comment 'MQTT Bird Alert'

sudo ufw enable
sudo ufw status verbose
```

`ufw enable` persists across reboots. Do not expose port 1883 to the public internet without TLS and stronger controls.

### 6.4 Option C — nftables (already on Pi OS, rarely configured)

Modern Pi OS (Bullseye and later) uses the **nftables** kernel framework. The `nftables` package may be present, but the **service is usually off** until you add rules in `/etc/nftables.conf` and run `sudo systemctl enable --now nftables`.

Only use this if you already manage nftables elsewhere. For a first Pi MQTT broker, **Option A or B** is simpler than writing nft rules by hand.

---

## 7. Values for ESP32 firmware

Firmware settings align with this server:

| Setting | Value |
|---------|--------|
| Broker host | Pi LAN IP (e.g. `192.168.1.50`) — defaults in `secrets.h`, overridable via on-device MQTT wizard |
| Port | `1883` |
| Username | `bird_alert` (or your chosen user) |
| Password | Same as `mosquitto_passwd` |
| Client ID | Unique per device from eFuse MAC (e.g. `bird-e831cd913a90`) |
| Subscribe / publish topic | `bird/events` |
| Keep alive | `60` seconds |
| QoS | `0` for alerts |

Store broker defaults in gitignored [`secrets.h`](../sketches/display_bringup/secrets.h) (copy from [`secrets.h.example`](../sketches/display_bringup/secrets.h.example)); see [mqtt-setup.md](mqtt-setup.md) for NVS and the on-device wizard.

---

## 8. TLS (optional)

For a trusted home LAN, plain MQTT on 1883 with username/password is usually sufficient. To encrypt traffic:

1. Generate or obtain certificates (e.g. self-signed CA for internal use).
2. Add a second listener on **8883** with `cafile`, `certfile`, and `keyfile` in `bird-alert.conf`.
3. Use `WiFiClientSecure` on the ESP32 with the CA certificate embedded or pinned.

Defer TLS until basic pub/sub works; ESP32 TLS adds flash/RAM cost and certificate management overhead.

---

## 9. Troubleshooting

| Symptom | Things to check |
|---------|------------------|
| Connection refused | `systemctl status mosquitto`; port 1883 listening: `ss -tlnp \| grep 1883` |
| Auth failed | Username/password match `passwd` file; `allow_anonymous false` is set |
| Works on Pi, not from ESP32 | Pi IP correct; same Wi‑Fi subnet; firewall rule; ESP32 on 2.4 GHz |
| Messages not received | Topic spelling exact (case-sensitive); subscriber connected before publish |
| Broker won’t start | `sudo journalctl -u mosquitto -n 50` for config syntax errors |

---

## 10. Useful commands

```bash
# Service
sudo systemctl restart mosquitto
sudo journalctl -u mosquitto -f

# Watch all bird topics
mosquitto_sub -h localhost -u bird_alert -P 'YOUR_PASSWORD' -t 'bird/#' -v
```

---

## Related

- [mqtt-setup.md](mqtt-setup.md) — on-device MQTT broker wizard and NVS keys
- [wifi-setup.md](wifi-setup.md) — on-device WiFi and NVS credentials
- [hardware.md](hardware.md) — ESP32 2.4 GHz Wi‑Fi only
