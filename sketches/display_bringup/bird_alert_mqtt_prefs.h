#pragma once

/**
 * MQTT broker settings: NVS overrides, then secrets.h / example defaults.
 */

#include <Arduino.h>
#include <Preferences.h>
#include <stdio.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef BIRD_ALERT_MQTT_HOST_DEFAULT
#define BIRD_ALERT_MQTT_HOST_DEFAULT ""
#endif
#ifndef BIRD_ALERT_MQTT_PORT_DEFAULT
#define BIRD_ALERT_MQTT_PORT_DEFAULT 1883
#endif
#ifndef BIRD_ALERT_MQTT_USER_DEFAULT
#define BIRD_ALERT_MQTT_USER_DEFAULT ""
#endif
#ifndef BIRD_ALERT_MQTT_PASS_DEFAULT
#define BIRD_ALERT_MQTT_PASS_DEFAULT ""
#endif

#define BIRD_ALERT_MQTT_ALERT_ID_MAX 48
#define BIRD_ALERT_MQTT_HOST_MAX 64
#define BIRD_ALERT_MQTT_USER_MAX 32
#define BIRD_ALERT_MQTT_PASS_MAX 64

static const char *BIRD_ALERT_MQTT_PREFS_NS = "bird_alert";
static const char *BIRD_ALERT_MQTT_HOST_KEY = "mqtt_host";
static const char *BIRD_ALERT_MQTT_PORT_KEY = "mqtt_port";
static const char *BIRD_ALERT_MQTT_USER_KEY = "mqtt_user";
static const char *BIRD_ALERT_MQTT_PASS_KEY = "mqtt_pass";

struct BirdAlertMqttConfig {
  char host[BIRD_ALERT_MQTT_HOST_MAX + 1];
  uint16_t port;
  char user[BIRD_ALERT_MQTT_USER_MAX + 1];
  char pass[BIRD_ALERT_MQTT_PASS_MAX + 1];
};

static inline void bird_alert_mqtt_config_set_defaults(BirdAlertMqttConfig *cfg) {
  if (cfg == nullptr) {
    return;
  }

  strncpy(cfg->host, BIRD_ALERT_MQTT_HOST_DEFAULT, BIRD_ALERT_MQTT_HOST_MAX);
  cfg->host[BIRD_ALERT_MQTT_HOST_MAX] = '\0';
  cfg->port = BIRD_ALERT_MQTT_PORT_DEFAULT;
  strncpy(cfg->user, BIRD_ALERT_MQTT_USER_DEFAULT, BIRD_ALERT_MQTT_USER_MAX);
  cfg->user[BIRD_ALERT_MQTT_USER_MAX] = '\0';
  strncpy(cfg->pass, BIRD_ALERT_MQTT_PASS_DEFAULT, BIRD_ALERT_MQTT_PASS_MAX);
  cfg->pass[BIRD_ALERT_MQTT_PASS_MAX] = '\0';
}

static inline bool bird_alert_mqtt_load(BirdAlertMqttConfig *cfg) {
  if (cfg == nullptr) {
    return false;
  }

  bird_alert_mqtt_config_set_defaults(cfg);

  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_MQTT_PREFS_NS, true)) {
    return cfg->host[0] != '\0';
  }

  const String storedHost = prefs.getString(BIRD_ALERT_MQTT_HOST_KEY, "");
  if (storedHost.length() > 0) {
    storedHost.toCharArray(cfg->host, sizeof(cfg->host));
  }

  const uint32_t storedPort = prefs.getUInt(BIRD_ALERT_MQTT_PORT_KEY, 0);
  if (storedPort > 0 && storedPort <= 65535) {
    cfg->port = (uint16_t)storedPort;
  }

  const String storedUser = prefs.getString(BIRD_ALERT_MQTT_USER_KEY, "");
  if (storedUser.length() > 0) {
    storedUser.toCharArray(cfg->user, sizeof(cfg->user));
  }

  const String storedPass = prefs.getString(BIRD_ALERT_MQTT_PASS_KEY, "");
  if (storedPass.length() > 0) {
    storedPass.toCharArray(cfg->pass, sizeof(cfg->pass));
  }

  prefs.end();
  return cfg->host[0] != '\0';
}

static inline void bird_alert_mqtt_save(const BirdAlertMqttConfig *cfg) {
  if (cfg == nullptr) {
    return;
  }

  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_MQTT_PREFS_NS, false)) {
    return;
  }

  prefs.putString(BIRD_ALERT_MQTT_HOST_KEY, cfg->host);
  prefs.putUInt(BIRD_ALERT_MQTT_PORT_KEY, cfg->port);
  prefs.putString(BIRD_ALERT_MQTT_USER_KEY, cfg->user);
  prefs.putString(BIRD_ALERT_MQTT_PASS_KEY, cfg->pass);
  prefs.end();

  Serial.printf("mqtt: saved broker %s:%u\n", cfg->host, (unsigned)cfg->port);
}

static inline void bird_alert_mqtt_clear(void) {
  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_MQTT_PREFS_NS, false)) {
    return;
  }

  prefs.remove(BIRD_ALERT_MQTT_HOST_KEY);
  prefs.remove(BIRD_ALERT_MQTT_PORT_KEY);
  prefs.remove(BIRD_ALERT_MQTT_USER_KEY);
  prefs.remove(BIRD_ALERT_MQTT_PASS_KEY);
  prefs.end();

  Serial.println("mqtt: cleared stored broker settings");
}
