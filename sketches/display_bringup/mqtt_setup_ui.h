#pragma once

/**
 * On-device MQTT broker setup UI.
 * Reuses touch keyboard patterns from wifi_setup_ui.h.
 */

#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"
#include "bird_alert_mqtt.h"
#include "bird_alert_mqtt_prefs.h"
#include "wifi_setup_ui.h"

static const int32_t MQTT_UI_ROW_H = 28;
static const int32_t MQTT_UI_FIELD_MAX = 4;

enum MqttSetupField : uint8_t {
  MQTT_FIELD_HOST = 0,
  MQTT_FIELD_PORT,
  MQTT_FIELD_USER,
  MQTT_FIELD_PASS,
};

static inline const char *mqtt_setup_ui_field_label(MqttSetupField field) {
  switch (field) {
    case MQTT_FIELD_HOST:
      return "Host";
    case MQTT_FIELD_PORT:
      return "Port";
    case MQTT_FIELD_USER:
      return "User";
    case MQTT_FIELD_PASS:
      return "Password";
    default:
      return "?";
  }
}

static inline void mqtt_setup_ui_get_field_value(const BirdAlertMqttConfig *cfg, MqttSetupField field, char *out,
                                                 size_t outLen) {
  if (cfg == nullptr || out == nullptr || outLen == 0) {
    return;
  }

  out[0] = '\0';
  switch (field) {
    case MQTT_FIELD_HOST:
      strncpy(out, cfg->host, outLen - 1);
      break;
    case MQTT_FIELD_PORT:
      snprintf(out, outLen, "%u", (unsigned)cfg->port);
      break;
    case MQTT_FIELD_USER:
      strncpy(out, cfg->user, outLen - 1);
      break;
    case MQTT_FIELD_PASS:
      strncpy(out, cfg->pass, outLen - 1);
      break;
    default:
      break;
  }
  out[outLen - 1] = '\0';
}

static inline void mqtt_setup_ui_set_field_value(BirdAlertMqttConfig *cfg, MqttSetupField field, const char *value) {
  if (cfg == nullptr || value == nullptr) {
    return;
  }

  switch (field) {
    case MQTT_FIELD_HOST:
      strncpy(cfg->host, value, BIRD_ALERT_MQTT_HOST_MAX);
      cfg->host[BIRD_ALERT_MQTT_HOST_MAX] = '\0';
      break;
    case MQTT_FIELD_PORT: {
      const unsigned long port = strtoul(value, nullptr, 10);
      if (port > 0 && port <= 65535) {
        cfg->port = (uint16_t)port;
      }
      break;
    }
    case MQTT_FIELD_USER:
      strncpy(cfg->user, value, BIRD_ALERT_MQTT_USER_MAX);
      cfg->user[BIRD_ALERT_MQTT_USER_MAX] = '\0';
      break;
    case MQTT_FIELD_PASS:
      strncpy(cfg->pass, value, BIRD_ALERT_MQTT_PASS_MAX);
      cfg->pass[BIRD_ALERT_MQTT_PASS_MAX] = '\0';
      break;
    default:
      break;
  }
}

static inline void mqtt_setup_ui_draw_form(LGFX_Elegoo28 &display, const BirdAlertMqttConfig *cfg) {
  const int32_t w = display.width();
  const int32_t h = display.height();
  const int32_t listY = WIFI_UI_HEADER_H;

  display.fillScreen(TFT_BLACK);
  wifi_setup_ui_draw_header(display, "MQTT Broker", false);

  char valueBuf[72];
  for (int i = 0; i < MQTT_UI_FIELD_MAX; i++) {
    const MqttSetupField field = (MqttSetupField)i;
    const int32_t rowY = listY + i * MQTT_UI_ROW_H;

    display.fillRect(0, rowY, w, MQTT_UI_ROW_H - 1, (i & 1) ? TFT_BLACK : 0x0841);
    display.setTextColor(TFT_CYAN);
    display.setTextDatum(textdatum_t::middle_left);
    display.drawString(mqtt_setup_ui_field_label(field), 4, rowY + MQTT_UI_ROW_H / 2);

    mqtt_setup_ui_get_field_value(cfg, field, valueBuf, sizeof(valueBuf));
    if (field == MQTT_FIELD_PASS) {
      char masked[72];
      size_t j = 0;
      for (size_t k = 0; valueBuf[k] != '\0' && j + 1 < sizeof(masked); k++) {
        masked[j++] = '*';
      }
      masked[j] = '\0';
      strncpy(valueBuf, masked, sizeof(valueBuf) - 1);
      valueBuf[sizeof(valueBuf) - 1] = '\0';
    }

    display.setTextColor(TFT_WHITE);
    display.setTextDatum(textdatum_t::middle_right);
    if (valueBuf[0] == '\0') {
      display.setTextColor(TFT_DARKGREY);
      display.drawString("(tap to set)", w - 4, rowY + MQTT_UI_ROW_H / 2);
    } else if (strlen(valueBuf) > 18) {
      char trunc[24];
      snprintf(trunc, sizeof(trunc), "%.15s...", valueBuf);
      display.drawString(trunc, w - 4, rowY + MQTT_UI_ROW_H / 2);
    } else {
      display.drawString(valueBuf, w - 4, rowY + MQTT_UI_ROW_H / 2);
    }
    display.setTextDatum(textdatum_t::top_left);
  }

  const int32_t footerY = h - WIFI_UI_FOOTER_H - 4;
  display.fillRect(4, footerY, w - 8, WIFI_UI_FOOTER_H, TFT_GREEN);
  display.setTextColor(TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Save && Connect", w / 2, footerY + WIFI_UI_FOOTER_H / 2);
  display.setTextDatum(textdatum_t::top_left);
}

static inline int mqtt_setup_ui_form_tap(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  const int32_t listY = WIFI_UI_HEADER_H;
  const int32_t footerY = screenH - WIFI_UI_FOOTER_H - 4;

  if (ty >= footerY && ty < footerY + WIFI_UI_FOOTER_H) {
    return 100;
  }

  if (ty >= listY && ty < listY + MQTT_UI_FIELD_MAX * MQTT_UI_ROW_H) {
    const int row = (ty - listY) / MQTT_UI_ROW_H;
    if (row >= 0 && row < MQTT_UI_FIELD_MAX) {
      return row;
    }
  }

  (void)tx;
  (void)screenW;
  return -1;
}

static inline bool mqtt_setup_ui_edit_field(LGFX_Elegoo28 &display, MqttSetupField field, char *buffer,
                                            size_t bufferLen) {
  const char *title = mqtt_setup_ui_field_label(field);
  bool shift = false;

  while (true) {
    display.fillScreen(TFT_BLACK);
    wifi_setup_ui_draw_header(display, title, true);

    display.setTextColor(TFT_WHITE);
    display.setTextSize(1);
    display.setCursor(4, WIFI_UI_HEADER_H + 4);
    display.print("Value: ");
    if (field == MQTT_FIELD_PASS) {
      for (size_t i = 0; buffer[i] != '\0'; i++) {
        display.print('*');
      }
    } else {
      display.print(buffer);
    }
    display.print('_');

    wifi_setup_ui_draw_keyboard(display, shift);

    int32_t tx = 0;
    int32_t ty = 0;
    wifi_setup_ui_wait_tap(display, &tx, &ty);

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
      return true;
    }
    if (ch != '\0') {
      if (field == MQTT_FIELD_PORT && (ch < '0' || ch > '9')) {
        continue;
      }
      const size_t len = strlen(buffer);
      if (len + 1 < bufferLen) {
        buffer[len] = ch;
        buffer[len + 1] = '\0';
      }
    }
  }
}

static inline bool mqtt_setup_ui_try_connect(LGFX_Elegoo28 &display, const BirdAlertMqttConfig *cfg) {
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Connecting MQTT...", display.width() / 2, display.height() / 2 - 8);
  display.drawString(cfg->host, display.width() / 2, display.height() / 2 + 8);
  display.setTextDatum(textdatum_t::top_left);

  bird_alert_mqtt_reconnect_after_config_change(cfg);

  if (bird_alert_mqtt_connected()) {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_GREEN);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("MQTT connected!", display.width() / 2, display.height() / 2);
    display.setTextDatum(textdatum_t::top_left);
    delay(1200);
    return true;
  }

  display.fillScreen(TFT_BLACK);
  wifi_setup_ui_draw_header(display, "MQTT failed", false);
  display.setTextColor(TFT_ORANGE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Check host, port, user, pass", display.width() / 2, display.height() / 2 - 8);
  display.drawString("Tap to continue", display.width() / 2, display.height() / 2 + 10);
  display.setTextDatum(textdatum_t::top_left);
  wifi_setup_ui_wait_tap(display, nullptr, nullptr);
  return false;
}

static inline bool mqtt_setup_ui_run(LGFX_Elegoo28 &display, BirdAlertMqttConfig *cfg) {
  if (cfg == nullptr) {
    return false;
  }

  bird_alert_mqtt_load(cfg);

  while (true) {
    mqtt_setup_ui_draw_form(display, cfg);

    int32_t tx = 0;
    int32_t ty = 0;
    wifi_setup_ui_wait_tap(display, &tx, &ty);

    const int action = mqtt_setup_ui_form_tap(tx, ty, display.width(), display.height());
    if (action >= 0 && action < MQTT_UI_FIELD_MAX) {
      char buffer[72] = {0};
      mqtt_setup_ui_get_field_value(cfg, (MqttSetupField)action, buffer, sizeof(buffer));
      if (mqtt_setup_ui_edit_field(display, (MqttSetupField)action, buffer, sizeof(buffer))) {
        mqtt_setup_ui_set_field_value(cfg, (MqttSetupField)action, buffer);
      }
      continue;
    }

    if (action == 100) {
      if (cfg->host[0] == '\0') {
        display.fillScreen(TFT_BLACK);
        display.setTextColor(TFT_ORANGE);
        display.setTextDatum(textdatum_t::middle_center);
        display.drawString("Host is required", display.width() / 2, display.height() / 2);
        display.setTextDatum(textdatum_t::top_left);
        delay(1200);
        continue;
      }

      bird_alert_mqtt_save(cfg);
      return mqtt_setup_ui_try_connect(display, cfg);
    }
  }
}
