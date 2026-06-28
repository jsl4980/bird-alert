#pragma once

/**
 * Bird Alert home screen: status dot, SPOT A BIRD / GOT IT, incoming alert banner.
 * Partial redraw via bird_alert_ui_refresh() dirty region mask.
 */

#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"
#include "bird_alert_device_id.h"
#include "bird_alert_icons.h"
#include "bird_alert_mqtt_prefs.h"
#include "bird_alert_nickname_prefs.h"
#include "bird_alert_wifi.h"
#include "wifi_setup_ui.h"

enum BirdAlertUiMode : uint8_t {
  BIRD_ALERT_UI_IDLE = 0,
  BIRD_ALERT_UI_SENT,
  BIRD_ALERT_UI_SHOWING,
  BIRD_ALERT_UI_ACKED,
};

enum BirdAlertUiDirty : uint8_t {
  BIRD_ALERT_UI_DIRTY_NONE = 0,
  BIRD_ALERT_UI_DIRTY_HEADER = 1 << 0,
  BIRD_ALERT_UI_DIRTY_MAIN = 1 << 1,
  BIRD_ALERT_UI_DIRTY_FOOTER = 1 << 2,
  BIRD_ALERT_UI_DIRTY_PULSE = 1 << 3,
  BIRD_ALERT_UI_DIRTY_FULL = 1 << 4,
};

static const int32_t BIRD_ALERT_UI_HEADER_H = 28;
static const int32_t BIRD_ALERT_UI_FOOTER_H = 26;
static const int32_t BIRD_ALERT_UI_MARGIN = 8;
static const int32_t BIRD_ALERT_UI_HERO_H = 92;
static const int32_t BIRD_ALERT_UI_HERO_GAP = 10;
static const int32_t BIRD_ALERT_UI_PRIMARY_BTN_H = 56;
static const int32_t BIRD_ALERT_UI_ACK_BTN_H = 48;
static const int32_t BIRD_ALERT_UI_BANNER_H = 56;
static const uint32_t BIRD_ALERT_UI_ACKED_MS = 1500;

static const uint16_t BIRD_ALERT_UI_PRIMARY_COLOR = 0xE124;

struct BirdAlertHomeView {
  BirdAlertWifiStatus wifi;
  bool mqttConnected;
  BirdAlertUiMode mode;
  char activeAlertId[BIRD_ALERT_MQTT_ALERT_ID_MAX];
  char activeFrom[BIRD_ALERT_DEVICE_ID_MAX];
  char ackFrom[BIRD_ALERT_DEVICE_ID_MAX];
  uint32_t activeAlertTs;
  uint8_t pulsePhase;
};

static inline int32_t bird_alert_ui_content_top(void) {
  return BIRD_ALERT_UI_HEADER_H;
}

static inline int32_t bird_alert_ui_content_bottom(int32_t screenH) {
  return screenH - BIRD_ALERT_UI_FOOTER_H;
}

static inline int32_t bird_alert_ui_primary_btn_w(int32_t screenW) {
  return screenW - BIRD_ALERT_UI_MARGIN * 2;
}

static inline void bird_alert_ui_content_rect(int32_t w, int32_t h, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->x = 0;
  out->y = BIRD_ALERT_UI_HEADER_H;
  out->w = w;
  out->h = bird_alert_ui_content_bottom(h) - BIRD_ALERT_UI_HEADER_H;
}

static inline int32_t bird_alert_ui_hero_y(void) {
  return BIRD_ALERT_UI_HEADER_H + 6;
}

static inline void bird_alert_ui_hero_rect(int32_t screenW, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->x = BIRD_ALERT_UI_MARGIN;
  out->y = bird_alert_ui_hero_y();
  out->w = screenW - BIRD_ALERT_UI_MARGIN * 2;
  out->h = BIRD_ALERT_UI_HERO_H;
}

static inline void bird_alert_ui_primary_button_rect(int32_t screenW, int32_t screenH, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->w = bird_alert_ui_primary_btn_w(screenW);
  out->x = BIRD_ALERT_UI_MARGIN;
  out->h = BIRD_ALERT_UI_PRIMARY_BTN_H;
  out->y = bird_alert_ui_hero_y() + BIRD_ALERT_UI_HERO_H + BIRD_ALERT_UI_HERO_GAP;
  (void)screenH;
}

static inline void bird_alert_ui_banner_rect(int32_t screenW, int32_t screenH, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->x = BIRD_ALERT_UI_MARGIN;
  out->y = bird_alert_ui_content_top() + 4;
  out->w = screenW - BIRD_ALERT_UI_MARGIN * 2;
  out->h = BIRD_ALERT_UI_BANNER_H;
}

static inline void bird_alert_ui_ack_button_rect(int32_t screenW, int32_t screenH, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->w = bird_alert_ui_primary_btn_w(screenW);
  out->x = BIRD_ALERT_UI_MARGIN;
  out->h = BIRD_ALERT_UI_ACK_BTN_H;
  out->y = bird_alert_ui_content_bottom(screenH) - out->h - 28;
}

static inline void bird_alert_ui_settings_button_rect(int32_t screenW, int32_t screenH, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  out->x = BIRD_ALERT_UI_MARGIN;
  out->y = screenH - BIRD_ALERT_UI_FOOTER_H + 4;
  out->w = 90;
  out->h = BIRD_ALERT_UI_FOOTER_H - 8;
  (void)screenW;
}

static inline void bird_alert_ui_pulse_rect(int32_t screenW, int32_t screenH, WifiSetupRect *out) {
  if (out == nullptr) {
    return;
  }
  WifiSetupRect alertBtn;
  bird_alert_ui_primary_button_rect(screenW, screenH, &alertBtn);
  const int32_t cx = screenW / 2;
  const int32_t cy = alertBtn.y + alertBtn.h + 18;
  out->x = cx - 20;
  out->y = cy - 6;
  out->w = 40;
  out->h = 12;
}

static inline int32_t bird_alert_ui_pulse_dots_y(int32_t screenW, int32_t screenH) {
  WifiSetupRect alertBtn;
  bird_alert_ui_primary_button_rect(screenW, screenH, &alertBtn);
  (void)screenW;
  return alertBtn.y + alertBtn.h + 18;
}

static inline uint16_t bird_alert_ui_connection_color(const BirdAlertHomeView &view) {
  if (!view.wifi.connected) {
    return TFT_RED;
  }
  if (!view.mqttConnected) {
    return TFT_ORANGE;
  }
  return TFT_GREEN;
}

static inline bool bird_alert_ui_can_alert(const BirdAlertHomeView &view) {
  return view.wifi.connected && view.mqttConnected && view.mode == BIRD_ALERT_UI_IDLE;
}

static inline void bird_alert_ui_draw_header(LGFX_Elegoo28 &display, int32_t w, const BirdAlertHomeView &view) {
  display.fillRect(0, 0, w, BIRD_ALERT_UI_HEADER_H - 1, 0x0010);
  display.drawFastHLine(0, BIRD_ALERT_UI_HEADER_H - 1, w, TFT_CYAN);

  display.setTextColor(TFT_CYAN);
  display.setTextSize(1);
  display.setTextDatum(textdatum_t::middle_left);
  display.drawString("Bird Alert", BIRD_ALERT_UI_MARGIN, BIRD_ALERT_UI_HEADER_H / 2);

  const uint16_t dotColor = bird_alert_ui_connection_color(view);
  bird_alert_icon_draw_connection_dot(display, w - BIRD_ALERT_UI_MARGIN - 4, BIRD_ALERT_UI_HEADER_H / 2, dotColor);

  display.setTextDatum(textdatum_t::top_left);
}

static inline void bird_alert_ui_draw_footer(LGFX_Elegoo28 &display, int32_t w, int32_t h) {
  display.drawFastHLine(0, h - BIRD_ALERT_UI_FOOTER_H, w, TFT_CYAN);

  WifiSetupRect settingsBtn;
  bird_alert_ui_settings_button_rect(w, h, &settingsBtn);
  display.fillRoundRect(settingsBtn.x, settingsBtn.y, settingsBtn.w, settingsBtn.h, 4, TFT_NAVY);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Settings", settingsBtn.x + settingsBtn.w / 2, settingsBtn.y + settingsBtn.h / 2);

  display.setTextColor(TFT_DARKGREY);
  display.setTextDatum(textdatum_t::middle_right);
  const char *name = bird_alert_display_name();
  if (strlen(name) > 18) {
    char trunc[22];
    snprintf(trunc, sizeof(trunc), "%.15s...", name);
    display.drawString(trunc, w - BIRD_ALERT_UI_MARGIN, h - BIRD_ALERT_UI_FOOTER_H / 2);
  } else {
    display.drawString(name, w - BIRD_ALERT_UI_MARGIN, h - BIRD_ALERT_UI_FOOTER_H / 2);
  }
  display.setTextDatum(textdatum_t::top_left);
}

static inline void bird_alert_ui_draw_waiting_dots(LGFX_Elegoo28 &display, int32_t cx, int32_t cy, uint8_t phase) {
  const uint16_t bright = TFT_ORANGE;
  const uint16_t dim = TFT_DARKGREY;
  for (int i = 0; i < 3; i++) {
    const uint16_t color = ((phase + i) % 3 == 0) ? bright : dim;
    display.fillCircle(cx - 12 + i * 12, cy, 3, color);
  }
}

static inline void bird_alert_ui_draw_main(LGFX_Elegoo28 &display, int32_t w, int32_t h,
                                           const BirdAlertHomeView &view) {
  if (view.mode == BIRD_ALERT_UI_SHOWING) {
    WifiSetupRect banner;
    bird_alert_ui_banner_rect(w, h, &banner);
    display.fillRoundRect(banner.x, banner.y, banner.w, banner.h, 6, TFT_MAROON);
    display.drawRoundRect(banner.x, banner.y, banner.w, banner.h, 6, TFT_RED);

    display.setTextColor(TFT_WHITE);
    display.setTextSize(2);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("BIRD SPOTTED!", banner.x + banner.w / 2, banner.y + 16);

    display.setTextSize(1);
    char line[48];
    snprintf(line, sizeof(line), "from %s", view.activeFrom);
    display.drawString(line, banner.x + banner.w / 2, banner.y + 38);
    display.setTextDatum(textdatum_t::top_left);

    WifiSetupRect ackBtn;
    bird_alert_ui_ack_button_rect(w, h, &ackBtn);
    display.fillRoundRect(ackBtn.x, ackBtn.y, ackBtn.w, ackBtn.h, 8, TFT_GREEN);
    display.drawRoundRect(ackBtn.x, ackBtn.y, ackBtn.w, ackBtn.h, 8, TFT_WHITE);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(2);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("GOT IT", ackBtn.x + ackBtn.w / 2, ackBtn.y + ackBtn.h / 2);
    display.setTextSize(1);
    display.setTextDatum(textdatum_t::top_left);

    display.setTextColor(TFT_DARKGREY);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Tap to let them know you're on your way", w / 2, ackBtn.y + ackBtn.h + 10);
    display.setTextDatum(textdatum_t::top_left);
    return;
  }

  if (view.mode == BIRD_ALERT_UI_ACKED) {
    display.setTextColor(TFT_GREEN);
    display.setTextSize(2);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Acknowledged!", w / 2, h / 2 - 24);

    display.setTextSize(1);
    char line[56];
    snprintf(line, sizeof(line), "by %s", view.ackFrom);
    display.setTextColor(TFT_WHITE);
    display.drawString(line, w / 2, h / 2 + 4);
    display.setTextDatum(textdatum_t::top_left);
    return;
  }

  const int32_t contentTop = bird_alert_ui_content_top();
  WifiSetupRect hero;
  bird_alert_ui_hero_rect(w, &hero);
  bird_alert_icon_draw_bird_scene(display, hero.x, hero.y, hero.w, hero.h);

  WifiSetupRect alertBtn;
  bird_alert_ui_primary_button_rect(w, h, &alertBtn);

  const bool sent = (view.mode == BIRD_ALERT_UI_SENT);
  const bool canAlert = bird_alert_ui_can_alert(view);
  const uint16_t btnColor = sent ? TFT_DARKGREY : (canAlert ? BIRD_ALERT_UI_PRIMARY_COLOR : TFT_DARKGREY);
  const uint16_t borderColor = sent ? TFT_ORANGE : (canAlert ? TFT_WHITE : TFT_ORANGE);

  display.fillRoundRect(alertBtn.x, alertBtn.y, alertBtn.w, alertBtn.h, 10, btnColor);
  display.drawRoundRect(alertBtn.x, alertBtn.y, alertBtn.w, alertBtn.h, 10, borderColor);

  display.setTextColor(sent ? TFT_ORANGE : TFT_WHITE);
  display.setTextSize(2);
  display.setTextDatum(textdatum_t::middle_center);
  if (sent) {
    display.drawString("Alert sent!", alertBtn.x + alertBtn.w / 2, alertBtn.y + alertBtn.h / 2 - 8);
    display.setTextSize(1);
    display.setTextColor(TFT_ORANGE);
    display.drawString("Waiting for response...", alertBtn.x + alertBtn.w / 2, alertBtn.y + alertBtn.h / 2 + 10);
    bird_alert_ui_draw_waiting_dots(display, w / 2, alertBtn.y + alertBtn.h + 18, view.pulsePhase);
  } else if (!canAlert) {
    display.setTextSize(1);
    display.setTextColor(TFT_ORANGE);
    display.drawString("Offline", alertBtn.x + alertBtn.w / 2, alertBtn.y + alertBtn.h / 2);
  } else {
    display.drawString("SPOT A BIRD!", alertBtn.x + alertBtn.w / 2, alertBtn.y + alertBtn.h / 2);
  }
  display.setTextSize(1);
  display.setTextDatum(textdatum_t::top_left);

  if (view.mode == BIRD_ALERT_UI_IDLE && canAlert) {
    display.setTextColor(TFT_DARKGREY);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Tap to alert everyone watching", w / 2, alertBtn.y + alertBtn.h + 8);
    display.setTextDatum(textdatum_t::top_left);
  } else if (view.mode == BIRD_ALERT_UI_IDLE && !canAlert) {
    display.setTextColor(TFT_DARKGREY);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Open Settings to connect", w / 2, alertBtn.y + alertBtn.h + 8);
    display.setTextDatum(textdatum_t::top_left);
  }
  (void)contentTop;
}

static inline void bird_alert_ui_draw_pulse(LGFX_Elegoo28 &display, int32_t w, int32_t h,
                                            const BirdAlertHomeView &view) {
  if (view.mode != BIRD_ALERT_UI_SENT) {
    return;
  }
  WifiSetupRect pulse;
  bird_alert_ui_pulse_rect(w, h, &pulse);
  display.fillRect(pulse.x, pulse.y, pulse.w, pulse.h, TFT_BLACK);
  bird_alert_ui_draw_waiting_dots(display, w / 2, bird_alert_ui_pulse_dots_y(w, h), view.pulsePhase);
}

static inline void bird_alert_ui_refresh(LGFX_Elegoo28 &display, const BirdAlertHomeView &view, uint8_t dirtyMask) {
  const int32_t w = display.width();
  const int32_t h = display.height();

  if (dirtyMask == BIRD_ALERT_UI_DIRTY_NONE) {
    return;
  }

  if (dirtyMask & BIRD_ALERT_UI_DIRTY_FULL) {
    display.fillScreen(TFT_BLACK);
    bird_alert_ui_draw_header(display, w, view);
    bird_alert_ui_draw_footer(display, w, h);
    bird_alert_ui_draw_main(display, w, h, view);
    display.drawRect(2, 2, w - 4, h - 4, TFT_CYAN);
    return;
  }

  if (dirtyMask & BIRD_ALERT_UI_DIRTY_HEADER) {
    display.fillRect(0, 0, w, BIRD_ALERT_UI_HEADER_H, TFT_BLACK);
    bird_alert_ui_draw_header(display, w, view);
  }

  if (dirtyMask & BIRD_ALERT_UI_DIRTY_FOOTER) {
    display.fillRect(0, h - BIRD_ALERT_UI_FOOTER_H, w, BIRD_ALERT_UI_FOOTER_H, TFT_BLACK);
    bird_alert_ui_draw_footer(display, w, h);
  }

  if (dirtyMask & BIRD_ALERT_UI_DIRTY_MAIN) {
    WifiSetupRect main;
    bird_alert_ui_content_rect(w, h, &main);
    display.fillRect(main.x, main.y, main.w, main.h, TFT_BLACK);
    bird_alert_ui_draw_main(display, w, h, view);
  }

  if (dirtyMask & BIRD_ALERT_UI_DIRTY_PULSE) {
    bird_alert_ui_draw_pulse(display, w, h, view);
  }
}

static inline void bird_alert_ui_draw_home(LGFX_Elegoo28 &display, const BirdAlertHomeView &view) {
  bird_alert_ui_refresh(display, view, BIRD_ALERT_UI_DIRTY_FULL);
}

static inline bool bird_alert_ui_hit_primary_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  WifiSetupRect btn;
  bird_alert_ui_primary_button_rect(screenW, screenH, &btn);
  return wifi_setup_ui_hit(btn, tx, ty);
}

static inline bool bird_alert_ui_hit_ack_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  WifiSetupRect btn;
  bird_alert_ui_ack_button_rect(screenW, screenH, &btn);
  return wifi_setup_ui_hit(btn, tx, ty);
}

static inline bool bird_alert_ui_hit_settings_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  WifiSetupRect btn;
  bird_alert_ui_settings_button_rect(screenW, screenH, &btn);
  return wifi_setup_ui_hit(btn, tx, ty);
}

static inline bool bird_alert_ui_hit_alert_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  return bird_alert_ui_hit_primary_button(tx, ty, screenW, screenH);
}

static inline bool bird_alert_ui_hit_wifi_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  return bird_alert_ui_hit_settings_button(tx, ty, screenW, screenH);
}
