#pragma once

/**
 * Persist WiFi credentials in ESP32 NVS (namespace bird_alert).
 * Survives reboot; cleared on re-flash.
 */

#include <Arduino.h>
#include <Preferences.h>

static const char *BIRD_ALERT_WIFI_PREFS_NS = "bird_alert";
static const char *BIRD_ALERT_WIFI_SSID_KEY = "wifi_ssid";
static const char *BIRD_ALERT_WIFI_PASS_KEY = "wifi_pass";

static inline bool bird_alert_wifi_load(char *ssid, size_t ssidLen, char *pass, size_t passLen) {
  if (ssid == nullptr || pass == nullptr || ssidLen == 0 || passLen == 0) {
    return false;
  }

  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_WIFI_PREFS_NS, true)) {
    return false;
  }

  const String storedSsid = prefs.getString(BIRD_ALERT_WIFI_SSID_KEY, "");
  if (storedSsid.length() == 0) {
    prefs.end();
    return false;
  }

  storedSsid.toCharArray(ssid, ssidLen);

  const String storedPass = prefs.getString(BIRD_ALERT_WIFI_PASS_KEY, "");
  prefs.end();

  storedPass.toCharArray(pass, passLen);
  return true;
}

static inline void bird_alert_wifi_save(const char *ssid, const char *pass) {
  if (ssid == nullptr) {
    return;
  }

  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_WIFI_PREFS_NS, false)) {
    return;
  }

  prefs.putString(BIRD_ALERT_WIFI_SSID_KEY, ssid);
  prefs.putString(BIRD_ALERT_WIFI_PASS_KEY, pass != nullptr ? pass : "");
  prefs.end();

  Serial.printf("wifi: saved credentials for \"%s\"\n", ssid);
}

static inline void bird_alert_wifi_clear(void) {
  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_WIFI_PREFS_NS, false)) {
    return;
  }

  prefs.remove(BIRD_ALERT_WIFI_SSID_KEY);
  prefs.remove(BIRD_ALERT_WIFI_PASS_KEY);
  prefs.end();

  Serial.println("wifi: cleared stored credentials");
}
