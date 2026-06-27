#pragma once

/**
 * Persist LovyanGFX touch calibration (uint16_t[8]) in ESP32 NVS.
 * Survives reboot; cleared on re-flash. Keyed by display rotation.
 */

#include <Arduino.h>
#include <Preferences.h>
#include "elegoo_28_display.h"

static const char *ELEGOO28_PREFS_NS = "bird_alert";
static const char *ELEGOO28_PREFS_ROT_KEY = "touch_cal_rot";

static inline String elegoo28_touch_cal_key(uint8_t index) {
  return String("touch_cal_") + index;
}

static inline bool elegoo28_touch_cal_load(uint16_t cal[8], uint8_t expected_rotation) {
  Preferences prefs;
  if (!prefs.begin(ELEGOO28_PREFS_NS, true)) {
    return false;
  }

  const uint8_t stored_rot = (uint8_t)prefs.getUChar(ELEGOO28_PREFS_ROT_KEY, 0xFF);
  if (stored_rot != expected_rotation) {
    prefs.end();
    return false;
  }

  bool ok = true;
  for (uint8_t i = 0; i < 8; i++) {
    const uint16_t v = (uint16_t)prefs.getUShort(elegoo28_touch_cal_key(i).c_str(), 0xFFFF);
    if (v == 0xFFFF) {
      ok = false;
      break;
    }
    cal[i] = v;
  }

  prefs.end();
  return ok;
}

static inline void elegoo28_touch_cal_save(const uint16_t cal[8], uint8_t rotation) {
  Preferences prefs;
  if (!prefs.begin(ELEGOO28_PREFS_NS, false)) {
    return;
  }

  prefs.putUChar(ELEGOO28_PREFS_ROT_KEY, rotation);
  for (uint8_t i = 0; i < 8; i++) {
    prefs.putUShort(elegoo28_touch_cal_key(i).c_str(), cal[i]);
  }

  prefs.end();
}

static inline void elegoo28_touch_cal_clear(void) {
  Preferences prefs;
  if (!prefs.begin(ELEGOO28_PREFS_NS, false)) {
    return;
  }

  prefs.remove(ELEGOO28_PREFS_ROT_KEY);
  for (uint8_t i = 0; i < 8; i++) {
    prefs.remove(elegoo28_touch_cal_key(i).c_str());
  }

  prefs.end();
}

static inline void elegoo28_touch_cal_print_serial(const uint16_t cal[8]) {
  Serial.println("touch cal[8] (Serial backup):");
  Serial.printf("  { %u, %u, %u, %u, %u, %u, %u, %u }\n",
                cal[0], cal[1], cal[2], cal[3], cal[4], cal[5], cal[6], cal[7]);
}
