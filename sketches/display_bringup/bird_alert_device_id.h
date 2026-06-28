#pragma once

/**
 * Hardware-derived device identity from ESP32 eFuse factory MAC (BLK0).
 * Stable across reboots; do not use WiFi STA MAC (interface offset may differ).
 */

#include <Arduino.h>
#include <esp_mac.h>
#include <stdio.h>

#define BIRD_ALERT_DEVICE_ID_MAX 20
#define BIRD_ALERT_MQTT_CLIENT_ID_MAX 24

static char g_bird_alert_device_id[BIRD_ALERT_DEVICE_ID_MAX] = {0};
static char g_bird_alert_mqtt_client_id[BIRD_ALERT_MQTT_CLIENT_ID_MAX] = {0};
static bool g_bird_alert_device_id_ready = false;

static inline void bird_alert_device_id_init(void) {
  if (g_bird_alert_device_id_ready) {
    return;
  }

  uint8_t mac[6] = {0};
  esp_efuse_mac_get_default(mac);

  snprintf(g_bird_alert_device_id, sizeof(g_bird_alert_device_id), "bird-%02x%02x%02x", mac[3], mac[4],
           mac[5]);

  snprintf(g_bird_alert_mqtt_client_id, sizeof(g_bird_alert_mqtt_client_id), "bird-%02x%02x%02x%02x%02x%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  g_bird_alert_device_id_ready = true;

  Serial.printf("device: id=%s client_id=%s\n", g_bird_alert_device_id, g_bird_alert_mqtt_client_id);
}

static inline const char *bird_alert_device_id(void) {
  if (!g_bird_alert_device_id_ready) {
    bird_alert_device_id_init();
  }
  return g_bird_alert_device_id;
}

static inline const char *bird_alert_mqtt_client_id(void) {
  if (!g_bird_alert_device_id_ready) {
    bird_alert_device_id_init();
  }
  return g_bird_alert_mqtt_client_id;
}
