#pragma once

/**
 * WiFi scan, connect, and status helpers for Bird Alert.
 */

#include <Arduino.h>
#include <WiFi.h>

#define BIRD_ALERT_WIFI_MAX_NETWORKS 20
#define BIRD_ALERT_WIFI_SSID_MAX 32
#define BIRD_ALERT_WIFI_PASS_MAX 64

struct BirdAlertWifiNetwork {
  char ssid[BIRD_ALERT_WIFI_SSID_MAX + 1];
  int32_t rssi;
  wifi_auth_mode_t auth;
  bool open;
};

struct BirdAlertWifiStatus {
  bool connected;
  char ssid[BIRD_ALERT_WIFI_SSID_MAX + 1];
  char ip[16];
  int32_t rssi;
};

static BirdAlertWifiNetwork bird_alert_wifi_scan_results[BIRD_ALERT_WIFI_MAX_NETWORKS];
static int bird_alert_wifi_scan_count = 0;

static inline bool bird_alert_wifi_is_open(wifi_auth_mode_t auth) {
  return auth == WIFI_AUTH_OPEN;
}

static inline int bird_alert_wifi_find_scan_index(const char *ssid) {
  for (int i = 0; i < bird_alert_wifi_scan_count; i++) {
    if (strcmp(bird_alert_wifi_scan_results[i].ssid, ssid) == 0) {
      return i;
    }
  }
  return -1;
}

static inline void bird_alert_wifi_upsert_scan_result(const char *ssid, int32_t rssi, wifi_auth_mode_t auth) {
  const int existing = bird_alert_wifi_find_scan_index(ssid);
  if (existing >= 0) {
    if (rssi > bird_alert_wifi_scan_results[existing].rssi) {
      bird_alert_wifi_scan_results[existing].rssi = rssi;
      bird_alert_wifi_scan_results[existing].auth = auth;
      bird_alert_wifi_scan_results[existing].open = bird_alert_wifi_is_open(auth);
    }
    return;
  }

  if (bird_alert_wifi_scan_count >= BIRD_ALERT_WIFI_MAX_NETWORKS) {
    return;
  }

  BirdAlertWifiNetwork *entry = &bird_alert_wifi_scan_results[bird_alert_wifi_scan_count++];
  strncpy(entry->ssid, ssid, BIRD_ALERT_WIFI_SSID_MAX);
  entry->ssid[BIRD_ALERT_WIFI_SSID_MAX] = '\0';
  entry->rssi = rssi;
  entry->auth = auth;
  entry->open = bird_alert_wifi_is_open(auth);
}

static inline void bird_alert_wifi_sort_scan_results(void) {
  for (int i = 0; i < bird_alert_wifi_scan_count - 1; i++) {
    for (int j = i + 1; j < bird_alert_wifi_scan_count; j++) {
      if (bird_alert_wifi_scan_results[j].rssi > bird_alert_wifi_scan_results[i].rssi) {
        const BirdAlertWifiNetwork tmp = bird_alert_wifi_scan_results[i];
        bird_alert_wifi_scan_results[i] = bird_alert_wifi_scan_results[j];
        bird_alert_wifi_scan_results[j] = tmp;
      }
    }
  }
}

static inline int bird_alert_wifi_scan(void) {
  bird_alert_wifi_scan_count = 0;

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, false);
  delay(100);

  Serial.println("wifi: scanning...");
  const int found = WiFi.scanNetworks(false, true);
  Serial.printf("wifi: scan found %d networks\n", found);

  if (found <= 0) {
    return 0;
  }

  for (int i = 0; i < found; i++) {
    const String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      continue;
    }
    bird_alert_wifi_upsert_scan_result(ssid.c_str(), WiFi.RSSI(i), WiFi.encryptionType(i));
  }

  bird_alert_wifi_sort_scan_results();
  return bird_alert_wifi_scan_count;
}

static inline bool bird_alert_wifi_connect(const char *ssid, const char *pass, uint32_t timeoutMs) {
  if (ssid == nullptr || ssid[0] == '\0') {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, false);
  delay(100);

  Serial.printf("wifi: connecting to \"%s\"...\n", ssid);
  if (pass != nullptr && pass[0] != '\0') {
    WiFi.begin(ssid, pass);
  } else {
    WiFi.begin(ssid);
  }

  const uint32_t start = millis();
  wl_status_t lastStatus = WL_IDLE_STATUS;

  while (millis() - start < timeoutMs) {
    const wl_status_t status = WiFi.status();
    if (status != lastStatus) {
      Serial.printf("wifi: status %d\n", status);
      lastStatus = status;
    }

    if (status == WL_CONNECTED) {
      Serial.printf("wifi: connected, IP=%s RSSI=%d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return true;
    }

    if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
      break;
    }

    delay(250);
  }

  Serial.printf("wifi: connect failed (status=%d)\n", WiFi.status());
  return false;
}

static inline void bird_alert_wifi_disconnect(void) {
  WiFi.disconnect(true, false);
  delay(100);
}

static inline BirdAlertWifiStatus bird_alert_wifi_status(void) {
  BirdAlertWifiStatus status = {};
  status.connected = (WiFi.status() == WL_CONNECTED);

  if (status.connected) {
    const String ssid = WiFi.SSID();
    ssid.toCharArray(status.ssid, sizeof(status.ssid));
    WiFi.localIP().toString().toCharArray(status.ip, sizeof(status.ip));
    status.rssi = WiFi.RSSI();
  }

  return status;
}

static inline const char *bird_alert_wifi_failure_reason(void) {
  switch (WiFi.status()) {
    case WL_NO_SSID_AVAIL:
      return "Network not found";
    case WL_CONNECT_FAILED:
      return "Wrong password";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Timeout";
  }
}
