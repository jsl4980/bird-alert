#pragma once

/**
 * On-device WiFi setup UI: network list, password keyboard, connect feedback.
 * Designed for 320x240 landscape (rotation 1).
 */

#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"
#include "bird_alert_wifi.h"
#include "bird_alert_wifi_prefs.h"

struct WifiSetupRect {
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

static const int32_t WIFI_UI_HEADER_H = 18;
static const int32_t WIFI_UI_ROW_H = 22;
static const int32_t WIFI_UI_LIST_ROWS = 6;
static const int32_t WIFI_UI_FOOTER_H = 22;
static const int32_t WIFI_UI_PASS_FIELD_H = 18;
static const int32_t WIFI_UI_KEY_H = 24;
static const int32_t WIFI_UI_HOME_BTN_W = 56;
static const int32_t WIFI_UI_HOME_BTN_H = 22;

static inline bool wifi_setup_ui_hit(const WifiSetupRect &rect, int32_t tx, int32_t ty) {
  return tx >= rect.x && tx < rect.x + rect.w && ty >= rect.y && ty < rect.y + rect.h;
}

static inline void wifi_setup_ui_wait_release(LGFX_Elegoo28 &display) {
  uint32_t idleSince = 0;
  while (true) {
    int32_t tx = -1;
    int32_t ty = -1;
    if (!display.getTouch(&tx, &ty)) {
      if (idleSince == 0) {
        idleSince = millis();
      } else if (millis() - idleSince >= 120) {
        return;
      }
    } else {
      idleSince = 0;
    }
    delay(10);
  }
}

static inline bool wifi_setup_ui_wait_tap(LGFX_Elegoo28 &display, int32_t *outX, int32_t *outY) {
  while (true) {
    int32_t tx = -1;
    int32_t ty = -1;
    if (display.getTouch(&tx, &ty)) {
      if (outX != nullptr) {
        *outX = tx;
      }
      if (outY != nullptr) {
        *outY = ty;
      }
      wifi_setup_ui_wait_release(display);
      return true;
    }
    delay(10);
  }
}

static inline void wifi_setup_ui_draw_header(LGFX_Elegoo28 &display, const char *title, bool showBack) {
  display.fillRect(0, 0, display.width(), WIFI_UI_HEADER_H, TFT_NAVY);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(1);
  display.setTextDatum(textdatum_t::middle_left);
  if (showBack) {
    display.drawString("< Back", 4, WIFI_UI_HEADER_H / 2);
  }
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString(title, display.width() / 2, WIFI_UI_HEADER_H / 2);
  display.setTextDatum(textdatum_t::top_left);
}

static inline void wifi_setup_ui_draw_signal_bars(LGFX_Elegoo28 &display, int32_t x, int32_t y, int32_t rssi) {
  int bars = 1;
  if (rssi >= -50) {
    bars = 4;
  } else if (rssi >= -60) {
    bars = 3;
  } else if (rssi >= -70) {
    bars = 2;
  }

  for (int i = 0; i < 4; i++) {
    const int32_t barH = 3 + i * 3;
    const int32_t barX = x + i * 5;
    const int32_t barY = y + (12 - barH);
    const uint16_t color = (i < bars) ? TFT_GREEN : TFT_DARKGREY;
    display.fillRect(barX, barY, 3, barH, color);
  }
}

static inline void wifi_setup_ui_draw_scanning(LGFX_Elegoo28 &display) {
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Scanning WiFi...", display.width() / 2, display.height() / 2);
  display.setTextDatum(textdatum_t::top_left);
}

static inline int wifi_setup_ui_draw_network_list(LGFX_Elegoo28 &display, int scrollOffset) {
  const int32_t w = display.width();
  const int32_t h = display.height();
  const int32_t listY = WIFI_UI_HEADER_H;
  const int32_t listH = WIFI_UI_ROW_H * WIFI_UI_LIST_ROWS;
  const int32_t footerY = listY + listH;

  display.fillScreen(TFT_BLACK);
  wifi_setup_ui_draw_header(display, "Select WiFi", false);

  if (bird_alert_wifi_scan_count == 0) {
    display.setTextColor(TFT_ORANGE);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("No networks found", w / 2, listY + listH / 2);
    display.setTextDatum(textdatum_t::top_left);
  } else {
    const int maxScroll = bird_alert_wifi_scan_count > WIFI_UI_LIST_ROWS
                              ? bird_alert_wifi_scan_count - WIFI_UI_LIST_ROWS
                              : 0;
    if (scrollOffset > maxScroll) {
      scrollOffset = maxScroll;
    }
    if (scrollOffset < 0) {
      scrollOffset = 0;
    }

    for (int row = 0; row < WIFI_UI_LIST_ROWS; row++) {
      const int index = scrollOffset + row;
      if (index >= bird_alert_wifi_scan_count) {
        break;
      }

      const BirdAlertWifiNetwork *net = &bird_alert_wifi_scan_results[index];
      const int32_t rowY = listY + row * WIFI_UI_ROW_H;

      display.fillRect(0, rowY, w, WIFI_UI_ROW_H - 1, (row & 1) ? TFT_BLACK : 0x0841);
      display.setTextColor(TFT_WHITE);
      display.setTextDatum(textdatum_t::middle_left);
      display.drawString(net->ssid, 28, rowY + WIFI_UI_ROW_H / 2);

      if (net->open) {
        display.setTextColor(TFT_CYAN);
        display.setTextDatum(textdatum_t::middle_right);
        display.drawString("(open)", w - 4, rowY + WIFI_UI_ROW_H / 2);
      }

      wifi_setup_ui_draw_signal_bars(display, 4, rowY + 4, net->rssi);
      display.setTextDatum(textdatum_t::top_left);
    }
  }

  display.fillRect(0, footerY, w, WIFI_UI_FOOTER_H, TFT_DARKGREY);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Rescan", w / 2, footerY + WIFI_UI_FOOTER_H / 2);
  display.setTextDatum(textdatum_t::top_left);

  return scrollOffset;
}

static inline int wifi_setup_ui_network_list_tap(LGFX_Elegoo28 &display, int32_t tx, int32_t ty,
                                                 int scrollOffset, char *outSsid, size_t ssidLen,
                                                 bool *outOpen) {
  const int32_t w = display.width();
  const int32_t listY = WIFI_UI_HEADER_H;
  const int32_t listH = WIFI_UI_ROW_H * WIFI_UI_LIST_ROWS;
  const int32_t footerY = listY + listH;

  if (ty >= footerY && ty < footerY + WIFI_UI_FOOTER_H) {
    return -1;
  }

  if (ty >= listY && ty < listY + listH) {
    const int row = (ty - listY) / WIFI_UI_ROW_H;
    const int index = scrollOffset + row;
    if (index >= 0 && index < bird_alert_wifi_scan_count) {
      const BirdAlertWifiNetwork *net = &bird_alert_wifi_scan_results[index];
      strncpy(outSsid, net->ssid, ssidLen - 1);
      outSsid[ssidLen - 1] = '\0';
      if (outOpen != nullptr) {
        *outOpen = net->open;
      }
      return 1;
    }
  }

  if (bird_alert_wifi_scan_count > WIFI_UI_LIST_ROWS) {
    if (tx >= w - 16) {
      return 2;
    }
    if (tx < 16) {
      return 3;
    }
  }

  (void)display;
  return 0;
}

struct WifiSetupKey {
  const char *label;
  char value;
  bool special;
  uint8_t specialId;
};

enum WifiSetupSpecialKey : uint8_t {
  WIFI_KEY_NONE = 0,
  WIFI_KEY_DEL,
  WIFI_KEY_SHIFT,
  WIFI_KEY_DONE,
  WIFI_KEY_BACK,
};

static inline void wifi_setup_ui_draw_password_field(LGFX_Elegoo28 &display, const char *ssid,
                                                     const char *password) {
  const int32_t w = display.width();
  const int32_t fieldY = WIFI_UI_HEADER_H + 2;

  display.fillRect(0, WIFI_UI_HEADER_H, w, WIFI_UI_PASS_FIELD_H + 4, TFT_BLACK);
  display.setTextColor(TFT_CYAN);
  display.setTextSize(1);
  display.setCursor(4, fieldY);
  if (strlen(ssid) > 28) {
    display.printf("%.25s...", ssid);
  } else {
    display.print(ssid);
  }

  display.setTextColor(TFT_WHITE);
  display.setCursor(4, fieldY + 10);
  display.print("Pass: ");
  for (size_t i = 0; password[i] != '\0'; i++) {
    display.print('*');
  }
  display.print('_');
}

static inline void wifi_setup_ui_draw_keyboard(LGFX_Elegoo28 &display, bool shift) {
  const int32_t w = display.width();
  const int32_t keyAreaY = WIFI_UI_HEADER_H + WIFI_UI_PASS_FIELD_H + 8;

  static const char *row1Lower = "1234567890";
  static const char *row2Lower = "qwertyuiop";
  static const char *row3Lower = "asdfghjkl";
  static const char *row4Lower = "zxcvbnm";

  static const char *row1Upper = "1234567890";
  static const char *row2Upper = "QWERTYUIOP";
  static const char *row3Upper = "ASDFGHJKL";
  static const char *row4Upper = "ZXCVBNM";

  const char *row1 = shift ? row1Upper : row1Lower;
  const char *row2 = shift ? row2Upper : row2Lower;
  const char *row3 = shift ? row3Upper : row3Lower;
  const char *row4 = shift ? row4Upper : row4Lower;

  display.fillRect(0, keyAreaY, w, display.height() - keyAreaY, TFT_BLACK);

  auto drawKeyRow = [&](const char *keys, int keyCount, int row, int startX, int totalW) {
    const int32_t keyW = totalW / keyCount;
    const int32_t rowY = keyAreaY + row * WIFI_UI_KEY_H;
    for (int i = 0; i < keyCount; i++) {
      const int32_t keyX = startX + i * keyW;
      display.drawRect(keyX, rowY, keyW - 1, WIFI_UI_KEY_H - 1, TFT_DARKGREY);
      display.setTextColor(TFT_WHITE);
      display.setTextDatum(textdatum_t::middle_center);
      char label[2] = {keys[i], '\0'};
      display.drawString(label, keyX + keyW / 2, rowY + WIFI_UI_KEY_H / 2);
    }
    display.setTextDatum(textdatum_t::top_left);
  };

  drawKeyRow(row1, 10, 0, 0, w);
  drawKeyRow(row2, 10, 1, 0, w);
  drawKeyRow(row3, 9, 2, 0, w - 40);

  const int32_t delX = w - 40;
  const int32_t delY = keyAreaY + 2 * WIFI_UI_KEY_H;
  display.fillRect(delX, delY, 38, WIFI_UI_KEY_H - 1, TFT_MAROON);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Del", delX + 19, delY + WIFI_UI_KEY_H / 2);

  const int32_t row4Y = keyAreaY + 3 * WIFI_UI_KEY_H;
  display.fillRect(0, row4Y, 44, WIFI_UI_KEY_H - 1, shift ? TFT_YELLOW : TFT_DARKGREY);
  display.setTextColor(shift ? TFT_BLACK : TFT_WHITE);
  display.drawString("Shift", 22, row4Y + WIFI_UI_KEY_H / 2);

  const int32_t keyW4 = (w - 44 - 50) / 7;
  for (int i = 0; i < 7; i++) {
    const int32_t keyX = 44 + i * keyW4;
    display.drawRect(keyX, row4Y, keyW4 - 1, WIFI_UI_KEY_H - 1, TFT_DARKGREY);
    display.setTextColor(TFT_WHITE);
    char label[2] = {row4[i], '\0'};
    display.drawString(label, keyX + keyW4 / 2, row4Y + WIFI_UI_KEY_H / 2);
  }

  display.fillRect(w - 50, row4Y, 48, WIFI_UI_KEY_H - 1, TFT_GREEN);
  display.setTextColor(TFT_BLACK);
  display.drawString("Done", w - 26, row4Y + WIFI_UI_KEY_H / 2);
  display.setTextDatum(textdatum_t::top_left);
}

static inline uint8_t wifi_setup_ui_keyboard_tap(LGFX_Elegoo28 &display, int32_t tx, int32_t ty, bool shift,
                                                 char *outChar) {
  const int32_t w = display.width();
  const int32_t keyAreaY = WIFI_UI_HEADER_H + WIFI_UI_PASS_FIELD_H + 8;

  if (ty < keyAreaY) {
    if (ty < WIFI_UI_HEADER_H && tx < 60) {
      return WIFI_KEY_BACK;
    }
    return WIFI_KEY_NONE;
  }

  const int row = (ty - keyAreaY) / WIFI_UI_KEY_H;
  if (row < 0 || row > 3) {
    return WIFI_KEY_NONE;
  }

  static const char *rowsLower[] = {"1234567890", "qwertyuiop", "asdfghjkl", "zxcvbnm"};
  static const char *rowsUpper[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
  const char *const *rows = shift ? rowsUpper : rowsLower;

  if (row == 0 || row == 1) {
    const int32_t keyW = w / 10;
    const int col = tx / keyW;
    if (col >= 0 && col < 10) {
      *outChar = rows[row][col];
      return WIFI_KEY_NONE;
    }
  }

  if (row == 2) {
    const int32_t keyW = (w - 40) / 9;
    if (tx < (w - 40)) {
      const int col = tx / keyW;
      if (col >= 0 && col < 9) {
        *outChar = rows[2][col];
        return WIFI_KEY_NONE;
      }
    } else {
      return WIFI_KEY_DEL;
    }
  }

  if (row == 3) {
    if (tx < 44) {
      return WIFI_KEY_SHIFT;
    }
    if (tx >= w - 50) {
      return WIFI_KEY_DONE;
    }
    const int32_t keyW = (w - 44 - 50) / 7;
    const int col = (tx - 44) / keyW;
    if (col >= 0 && col < 7) {
      *outChar = rows[3][col];
      return WIFI_KEY_NONE;
    }
  }

  (void)display;
  return WIFI_KEY_NONE;
}

static inline bool wifi_setup_ui_run_password(LGFX_Elegoo28 &display, const char *ssid, char *password,
                                              size_t passMax) {
  bool shift = false;
  password[0] = '\0';

  while (true) {
    display.fillScreen(TFT_BLACK);
    wifi_setup_ui_draw_header(display, "Enter password", true);
    wifi_setup_ui_draw_password_field(display, ssid, password);
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
      const size_t len = strlen(password);
      if (len > 0) {
        password[len - 1] = '\0';
      }
      continue;
    }
    if (action == WIFI_KEY_DONE) {
      return true;
    }
    if (ch != '\0') {
      const size_t len = strlen(password);
      if (len + 1 < passMax) {
        password[len] = ch;
        password[len + 1] = '\0';
      }
    }
  }
}

static inline bool wifi_setup_ui_try_connect(LGFX_Elegoo28 &display, const char *ssid, const char *password) {
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString("Connecting to", display.width() / 2, display.height() / 2 - 12);
  display.drawString(ssid, display.width() / 2, display.height() / 2 + 4);
  display.setTextDatum(textdatum_t::top_left);

  const bool ok = bird_alert_wifi_connect(ssid, password, 15000);

  if (ok) {
    bird_alert_wifi_save(ssid, password);
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_GREEN);
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Connected!", display.width() / 2, display.height() / 2 - 8);
    display.setTextColor(TFT_WHITE);
    display.drawString(WiFi.localIP().toString().c_str(), display.width() / 2, display.height() / 2 + 10);
    display.setTextDatum(textdatum_t::top_left);
    delay(1500);
    return true;
  }

  display.fillScreen(TFT_BLACK);
  wifi_setup_ui_draw_header(display, "Connect failed", false);
  display.setTextColor(TFT_ORANGE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString(bird_alert_wifi_failure_reason(), display.width() / 2, display.height() / 2 - 20);
  display.setTextColor(TFT_WHITE);
  display.drawString("Tap Retry or Back", display.width() / 2, display.height() / 2 + 4);

  const int32_t btnY = display.height() - WIFI_UI_FOOTER_H - 4;
  const int32_t btnW = 80;
  const int32_t midX = display.width() / 2;
  display.fillRect(midX - btnW - 8, btnY, btnW, WIFI_UI_FOOTER_H, TFT_MAROON);
  display.fillRect(midX + 8, btnY, btnW, WIFI_UI_FOOTER_H, TFT_DARKGREY);
  display.drawString("Back", midX - btnW / 2 - 8, btnY + WIFI_UI_FOOTER_H / 2);
  display.drawString("Retry", midX + btnW / 2 + 8, btnY + WIFI_UI_FOOTER_H / 2);
  display.setTextDatum(textdatum_t::top_left);

  while (true) {
    int32_t tx = 0;
    int32_t ty = 0;
    wifi_setup_ui_wait_tap(display, &tx, &ty);

    if (ty >= btnY && ty < btnY + WIFI_UI_FOOTER_H) {
      if (tx < midX) {
        return false;
      }
      return wifi_setup_ui_try_connect(display, ssid, password);
    }
  }
}

static inline bool wifi_setup_ui_run(LGFX_Elegoo28 &display) {
  int scrollOffset = 0;

  while (true) {
    wifi_setup_ui_draw_scanning(display);
    bird_alert_wifi_scan();
    scrollOffset = wifi_setup_ui_draw_network_list(display, scrollOffset);

    while (true) {
      int32_t tx = 0;
      int32_t ty = 0;
      wifi_setup_ui_wait_tap(display, &tx, &ty);

      char ssid[BIRD_ALERT_WIFI_SSID_MAX + 1] = {0};
      bool open = false;
      const int action = wifi_setup_ui_network_list_tap(display, tx, ty, scrollOffset, ssid, sizeof(ssid), &open);

      if (action == -1) {
        break;
      }
      if (action == 2) {
        scrollOffset++;
        scrollOffset = wifi_setup_ui_draw_network_list(display, scrollOffset);
        continue;
      }
      if (action == 3) {
        scrollOffset--;
        scrollOffset = wifi_setup_ui_draw_network_list(display, scrollOffset);
        continue;
      }
      if (action != 1) {
        continue;
      }

      char password[BIRD_ALERT_WIFI_PASS_MAX + 1] = {0};
      if (!open) {
        if (!wifi_setup_ui_run_password(display, ssid, password, sizeof(password))) {
          scrollOffset = wifi_setup_ui_draw_network_list(display, scrollOffset);
          continue;
        }
      }

      if (wifi_setup_ui_try_connect(display, ssid, password)) {
        return true;
      }
    }
  }
}

static inline bool wifi_setup_ui_hit_home_wifi_button(int32_t tx, int32_t ty, int32_t screenW, int32_t screenH) {
  const WifiSetupRect btn = {screenW - WIFI_UI_HOME_BTN_W - 4, screenH - WIFI_UI_HOME_BTN_H - 4,
                             WIFI_UI_HOME_BTN_W, WIFI_UI_HOME_BTN_H};
  return wifi_setup_ui_hit(btn, tx, ty);
}
