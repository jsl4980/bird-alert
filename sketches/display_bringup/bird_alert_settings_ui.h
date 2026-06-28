#pragma once

/**
 * Settings hub: nickname, Wi-Fi, MQTT broker, device ID.
 */

#include <string.h>

#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"
#include "bird_alert_device_id.h"
#include "bird_alert_icons.h"
#include "bird_alert_mqtt.h"
#include "bird_alert_mqtt_prefs.h"
#include "bird_alert_nickname_prefs.h"
#include "bird_alert_wifi.h"
#include "mqtt_setup_ui.h"
#include "wifi_setup_ui.h"

enum BirdAlertSettingsRow : uint8_t {
  BIRD_ALERT_SETTINGS_NAME = 0,
  BIRD_ALERT_SETTINGS_WIFI,
  BIRD_ALERT_SETTINGS_MQTT,
  BIRD_ALERT_SETTINGS_DEVICE_ID,
  BIRD_ALERT_SETTINGS_ROW_COUNT,
};

static const int32_t BIRD_ALERT_SETTINGS_ROW_H = 46;
static const int32_t BIRD_ALERT_SETTINGS_ROW_GAP = 4;
static const int32_t BIRD_ALERT_SETTINGS_LIST_TOP = WIFI_UI_HEADER_H;
static const int32_t BIRD_ALERT_SETTINGS_LABEL_X = 8;

static inline int32_t bird_alert_settings_row_y(int rowIndex) {
  return BIRD_ALERT_SETTINGS_LIST_TOP +
         rowIndex * (BIRD_ALERT_SETTINGS_ROW_H + BIRD_ALERT_SETTINGS_ROW_GAP);
}

static inline const char *bird_alert_settings_row_label(BirdAlertSettingsRow row) {
  switch (row) {
    case BIRD_ALERT_SETTINGS_NAME:
      return "My name";
    case BIRD_ALERT_SETTINGS_WIFI:
      return "Wi-Fi";
    case BIRD_ALERT_SETTINGS_MQTT:
      return "MQTT broker";
    case BIRD_ALERT_SETTINGS_DEVICE_ID:
      return "Device ID";
    default:
      return "?";
  }
}

static inline void bird_alert_settings_row_value(BirdAlertSettingsRow row, const BirdAlertWifiStatus &wifi,
                                                 const BirdAlertMqttConfig *mqttCfg, char *out, size_t outLen) {
  if (out == nullptr || outLen == 0) {
    return;
  }
  out[0] = '\0';

  switch (row) {
    case BIRD_ALERT_SETTINGS_NAME: {
      const char *nick = bird_alert_nickname();
      if (nick[0] != '\0') {
        strncpy(out, nick, outLen - 1);
      } else {
        strncpy(out, "Not set", outLen - 1);
      }
      break;
    }
    case BIRD_ALERT_SETTINGS_WIFI:
      if (wifi.connected) {
        strncpy(out, wifi.ssid, outLen - 1);
      } else {
        strncpy(out, "Not connected", outLen - 1);
      }
      break;
    case BIRD_ALERT_SETTINGS_MQTT:
      if (mqttCfg != nullptr && mqttCfg->host[0] != '\0') {
        strncpy(out, mqttCfg->host, outLen - 1);
      } else {
        strncpy(out, "Not set", outLen - 1);
      }
      break;
    case BIRD_ALERT_SETTINGS_DEVICE_ID:
      strncpy(out, bird_alert_device_id(), outLen - 1);
      break;
    default:
      break;
  }
  out[outLen - 1] = '\0';
}

static inline void bird_alert_settings_draw(LGFX_Elegoo28 &display, const BirdAlertWifiStatus &wifi,
                                            bool mqttConnected, const BirdAlertMqttConfig *mqttCfg) {
  const int32_t w = display.width();
  const int32_t h = display.height();

  display.fillScreen(TFT_BLACK);
  wifi_setup_ui_draw_header(display, "Settings", true);

  char valueBuf[48];
  for (int i = 0; i < BIRD_ALERT_SETTINGS_ROW_COUNT; i++) {
    const BirdAlertSettingsRow row = (BirdAlertSettingsRow)i;
    const int32_t rowY = bird_alert_settings_row_y(i);
    const int32_t textY = rowY + BIRD_ALERT_SETTINGS_ROW_H / 2;

    display.fillRect(0, rowY, w, BIRD_ALERT_SETTINGS_ROW_H, (i & 1) ? TFT_BLACK : 0x0841);
    display.setTextColor(TFT_CYAN);
    display.setTextDatum(textdatum_t::middle_left);
    display.drawString(bird_alert_settings_row_label(row), BIRD_ALERT_SETTINGS_LABEL_X, textY);

    bird_alert_settings_row_value(row, wifi, mqttCfg, valueBuf, sizeof(valueBuf));

    display.setTextColor(TFT_WHITE);
    if (row == BIRD_ALERT_SETTINGS_DEVICE_ID) {
      display.setTextColor(TFT_DARKGREY);
    } else if (strcmp(valueBuf, "Not set") == 0 || strcmp(valueBuf, "Not connected") == 0) {
      display.setTextColor(TFT_DARKGREY);
    }

    display.setTextDatum(textdatum_t::middle_right);
    if (row == BIRD_ALERT_SETTINGS_DEVICE_ID) {
      display.drawString(valueBuf, w - 8, textY);
    } else if (strlen(valueBuf) > 16) {
      char trunc[22];
      snprintf(trunc, sizeof(trunc), "%.13s...", valueBuf);
      display.drawString(trunc, w - 16, textY);
      display.setTextColor(TFT_DARKGREY);
      display.drawString(">", w - 6, textY);
    } else {
      display.drawString(valueBuf, w - 16, textY);
      display.setTextColor(TFT_DARKGREY);
      display.drawString(">", w - 6, textY);
    }
    display.setTextDatum(textdatum_t::top_left);
  }

  const int32_t stripY = h - WIFI_UI_FOOTER_H - 4;
  display.fillRect(0, stripY, w, WIFI_UI_FOOTER_H, 0x0841);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_left);
  display.drawString("WiFi", 8, stripY + WIFI_UI_FOOTER_H / 2);
  bird_alert_icon_draw_connection_dot(display, 36, stripY + WIFI_UI_FOOTER_H / 2,
                                      wifi.connected ? TFT_GREEN : TFT_RED);
  display.drawString("MQTT", 56, stripY + WIFI_UI_FOOTER_H / 2);
  bird_alert_icon_draw_connection_dot(display, 92, stripY + WIFI_UI_FOOTER_H / 2,
                                      mqttConnected ? TFT_GREEN : TFT_ORANGE);
  display.setTextDatum(textdatum_t::top_left);
}

static inline int bird_alert_settings_tap(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  if (ty < WIFI_UI_HEADER_H && tx < 60) {
    return -2;
  }

  for (int i = 0; i < BIRD_ALERT_SETTINGS_ROW_COUNT; i++) {
    const int32_t rowY = bird_alert_settings_row_y(i);
    if (ty >= rowY && ty < rowY + BIRD_ALERT_SETTINGS_ROW_H) {
      return i;
    }
  }

  (void)screenW;
  (void)screenH;
  return -1;
}

static inline bool bird_alert_settings_edit_nickname(LGFX_Elegoo28 &display) {
  char buffer[BIRD_ALERT_NICKNAME_MAX + 1] = {0};
  strncpy(buffer, bird_alert_nickname(), sizeof(buffer) - 1);
  bool shift = false;

  while (true) {
    display.fillScreen(TFT_BLACK);
    wifi_setup_ui_draw_header(display, "My name", true);

    display.setTextColor(TFT_WHITE);
    display.setTextSize(1);
    display.setCursor(4, WIFI_UI_HEADER_H + 4);
    display.print("Name: ");
    display.print(buffer);
    display.print('_');

    display.setTextColor(TFT_DARKGREY);
    display.setCursor(4, WIFI_UI_HEADER_H + 16);
    display.print("Leave empty for device ID");

    wifi_setup_ui_draw_keyboard(display, shift);

    int32_t tx = 0;
    int32_t ty = 0;
    wifi_setup_ui_wait_tap(display, &tx, &ty);

    if (ty < WIFI_UI_HEADER_H && tx < 60) {
      return false;
    }

    char ch = '\0';
    const uint8_t action = wifi_setup_ui_keyboard_tap(display, tx, ty, shift, &ch);

    if (action == WIFI_KEY_BACK) {
      return false;
    }
    if (action == WIFI_KEY_SHIFT) {
      shift = !shift;
      continue;
    }
    if (action == WIFI_KEY_DEL) {
      const size_t len = strlen(buffer);
      if (len > 0) {
        buffer[len - 1] = '\0';
      }
      continue;
    }
    if (action == WIFI_KEY_DONE) {
      bird_alert_nickname_save(buffer);
      return true;
    }
    if (ch != '\0') {
      const size_t len = strlen(buffer);
      if (len + 1 <= BIRD_ALERT_NICKNAME_MAX) {
        buffer[len] = ch;
        buffer[len + 1] = '\0';
      }
    }
  }
}

static inline void bird_alert_settings_ui_run(LGFX_Elegoo28 &display, BirdAlertMqttConfig *mqttCfg) {
  while (true) {
    const BirdAlertWifiStatus wifi = bird_alert_wifi_status();
    const bool mqttConnected = bird_alert_mqtt_connected();
    bird_alert_settings_draw(display, wifi, mqttConnected, mqttCfg);

    int32_t tx = 0;
    int32_t ty = 0;
    wifi_setup_ui_wait_tap(display, &tx, &ty);

    const int action = bird_alert_settings_tap(tx, ty, display.width(), display.height());
    if (action == -2) {
      return;
    }
    if (action == BIRD_ALERT_SETTINGS_NAME) {
      bird_alert_settings_edit_nickname(display);
      continue;
    }
    if (action == BIRD_ALERT_SETTINGS_WIFI) {
      wifi_setup_ui_run(display);
      continue;
    }
    if (action == BIRD_ALERT_SETTINGS_MQTT && mqttCfg != nullptr) {
      mqtt_setup_ui_run(display, mqttCfg);
      continue;
    }
  }
}
