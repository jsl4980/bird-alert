#pragma once

/**
 * Optional friendly device name stored in NVS (display only; MQTT from stays device id).
 */

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

#include "bird_alert_device_id.h"

#define BIRD_ALERT_NICKNAME_MAX 24

static const char *BIRD_ALERT_NICKNAME_PREFS_NS = "bird_alert";
static const char *BIRD_ALERT_NICKNAME_KEY = "device_nickname";

static char g_bird_alert_nickname[BIRD_ALERT_NICKNAME_MAX + 1] = {0};
static bool g_bird_alert_nickname_loaded = false;

static inline void bird_alert_nickname_load(void) {
  if (g_bird_alert_nickname_loaded) {
    return;
  }

  g_bird_alert_nickname[0] = '\0';

  Preferences prefs;
  if (prefs.begin(BIRD_ALERT_NICKNAME_PREFS_NS, true)) {
    const String stored = prefs.getString(BIRD_ALERT_NICKNAME_KEY, "");
    if (stored.length() > 0) {
      stored.toCharArray(g_bird_alert_nickname, sizeof(g_bird_alert_nickname));
    }
    prefs.end();
  }

  g_bird_alert_nickname_loaded = true;
}

static inline const char *bird_alert_nickname(void) {
  bird_alert_nickname_load();
  return g_bird_alert_nickname;
}

static inline const char *bird_alert_display_name(void) {
  bird_alert_nickname_load();
  if (g_bird_alert_nickname[0] != '\0') {
    return g_bird_alert_nickname;
  }
  return bird_alert_device_id();
}

static inline void bird_alert_nickname_save(const char *nickname) {
  if (nickname == nullptr) {
    return;
  }

  strncpy(g_bird_alert_nickname, nickname, BIRD_ALERT_NICKNAME_MAX);
  g_bird_alert_nickname[BIRD_ALERT_NICKNAME_MAX] = '\0';
  g_bird_alert_nickname_loaded = true;

  Preferences prefs;
  if (!prefs.begin(BIRD_ALERT_NICKNAME_PREFS_NS, false)) {
    return;
  }

  if (g_bird_alert_nickname[0] == '\0') {
    prefs.remove(BIRD_ALERT_NICKNAME_KEY);
  } else {
    prefs.putString(BIRD_ALERT_NICKNAME_KEY, g_bird_alert_nickname);
  }
  prefs.end();
}

static inline void bird_alert_nickname_clear(void) {
  g_bird_alert_nickname[0] = '\0';
  g_bird_alert_nickname_loaded = true;

  Preferences prefs;
  if (prefs.begin(BIRD_ALERT_NICKNAME_PREFS_NS, false)) {
    prefs.remove(BIRD_ALERT_NICKNAME_KEY);
    prefs.end();
  }
}
