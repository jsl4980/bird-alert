#pragma once

/**
 * Bird icon, hero panel, and connection dot for home screen.
 */

#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"

static const uint8_t BIRD_ALERT_ICON_BIRD_W = 56;
static const uint8_t BIRD_ALERT_ICON_BIRD_H = 48;

static const uint16_t BIRD_ALERT_ICON_HERO_BG = 0x0186;
static const uint16_t BIRD_ALERT_ICON_HERO_BORDER = 0x04FF;
static const uint16_t BIRD_ALERT_ICON_BIRD_BODY = 0xFE60;
static const uint16_t BIRD_ALERT_ICON_BIRD_WING = 0xC040;
static const uint16_t BIRD_ALERT_ICON_BIRD_BEAK = 0xFD20;

static inline void bird_alert_icon_draw_hero_panel(LGFX_Elegoo28 &display, int32_t x, int32_t y, int32_t w,
                                                 int32_t h) {
  display.fillRoundRect(x, y, w, h, 10, BIRD_ALERT_ICON_HERO_BG);
  display.drawRoundRect(x, y, w, h, 10, BIRD_ALERT_ICON_HERO_BORDER);
  display.drawFastHLine(x + 12, y + h - 18, w - 24, 0x0320);
  display.drawFastHLine(x + 12, y + h - 17, w - 24, 0x0210);
  // Sun (top-left) — kept away from header connection dot (top-right of screen)
  display.fillCircle(x + 24, y + 20, 9, 0x2100);
  display.fillCircle(x + 24, y + 20, 5, 0xFFE0);
}

static inline void bird_alert_icon_draw_bird_large(LGFX_Elegoo28 &display, int32_t x, int32_t y) {
  display.drawFastHLine(x + 4, y + 40, 48, 0x0400);
  display.drawFastHLine(x + 4, y + 41, 48, 0x0200);
  display.fillTriangle(x + 2, y + 28, x + 14, y + 24, x + 12, y + 36, BIRD_ALERT_ICON_BIRD_WING);
  display.fillEllipse(x + 26, y + 28, 14, 11, BIRD_ALERT_ICON_BIRD_BODY);
  display.fillEllipse(x + 22, y + 30, 9, 6, BIRD_ALERT_ICON_BIRD_WING);
  display.fillCircle(x + 38, y + 20, 9, BIRD_ALERT_ICON_BIRD_BODY);
  display.fillCircle(x + 41, y + 18, 2, TFT_WHITE);
  display.fillCircle(x + 42, y + 18, 1, TFT_BLACK);
  display.fillTriangle(x + 46, y + 20, x + 54, y + 19, x + 46, y + 23, BIRD_ALERT_ICON_BIRD_BEAK);
  display.drawFastVLine(x + 22, y + 36, 5, 0x0400);
  display.drawFastVLine(x + 30, y + 36, 5, 0x0400);
}

static inline void bird_alert_icon_draw_bird_scene(LGFX_Elegoo28 &display, int32_t panelX, int32_t panelY,
                                                   int32_t panelW, int32_t panelH) {
  bird_alert_icon_draw_hero_panel(display, panelX, panelY, panelW, panelH);

  const int32_t birdX = panelX + (panelW - BIRD_ALERT_ICON_BIRD_W) / 2;
  const int32_t birdY = panelY + (panelH - BIRD_ALERT_ICON_BIRD_H) / 2 - 4;
  bird_alert_icon_draw_bird_large(display, birdX, birdY);

  display.setTextColor(0x4AC0);
  display.setTextSize(1);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("On the lookout", panelX + panelW / 2, panelY + panelH - 10);
  display.setTextDatum(textdatum_t::top_left);
}

static inline void bird_alert_icon_draw_connection_dot(LGFX_Elegoo28 &display, int32_t cx, int32_t cy,
                                                       uint16_t color) {
  display.fillCircle(cx, cy, 4, color);
}
