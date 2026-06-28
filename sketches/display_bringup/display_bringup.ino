/*
 * Bird Alert — display + touch bring-up (LovyanGFX)
 *
 * TFT:  HSPI 14/13/12   Touch: VSPI 25/32/39  CS33  IRQ36
 *
 * First boot: corner calibration wizard → NVS. Then WiFi setup (if enabled).
 * Library: LovyanGFX (lovyan03)
 * Serial: 115200 — press EN with monitor open if you miss boot lines.
 */
#include <LovyanGFX.hpp>
#include "LGFX_Elegoo28.hpp"
#include "elegoo28_touch_cal.h"
#if BIRD_ALERT_WIFI_ENABLED
#include "bird_alert_wifi_prefs.h"
#include "bird_alert_wifi.h"
#include "bird_alert_wifi_events.h"
#include "wifi_setup_ui.h"
#include "bird_alert_device_id.h"
#include "bird_alert_ui.h"
#if BIRD_ALERT_MQTT_ENABLED
#include "bird_alert_mqtt_prefs.h"
#include "bird_alert_mqtt.h"
#include "mqtt_setup_ui.h"
#include "bird_alert_settings_ui.h"
#include "bird_alert_nickname_prefs.h"
#endif
#endif

LGFX_Elegoo28 display;

static const int CROSS_HALF = 8;
static const int CORNER_MARGIN = 20;
static const int CORNER_TARGET_R = 10;

#if BIRD_ALERT_WIFI_ENABLED
static BirdAlertHomeView g_homeView = {};
static uint8_t g_homeDirtyMask = BIRD_ALERT_UI_DIRTY_FULL;
#if BIRD_ALERT_MQTT_ENABLED
static BirdAlertMqttConfig g_mqttConfig = {};
static uint32_t g_ackedAtMs = 0;
#endif
#endif

struct CornerTarget {
  const char *label;
  int32_t x;
  int32_t y;
};

static void drawTouchMarker(int32_t x, int32_t y, uint16_t color) {
  display.drawFastHLine(x - CROSS_HALF, y, CROSS_HALF * 2 + 1, color);
  display.drawFastVLine(x, y - CROSS_HALF, CROSS_HALF * 2 + 1, color);
}

static void drawCornerTarget(const CornerTarget &target, bool active) {
  const uint16_t ring = active ? TFT_YELLOW : TFT_DARKGREY;
  display.fillCircle(target.x, target.y, CORNER_TARGET_R, TFT_RED);
  display.drawCircle(target.x, target.y, CORNER_TARGET_R + 2, ring);
  display.setTextColor(TFT_WHITE);
  display.setTextDatum(textdatum_t::middle_center);
  display.drawString(target.label, target.x, target.y);
  display.setTextDatum(textdatum_t::top_left);
}

static void logTouchPinConfig(void) {
  Serial.println("touch pins (VSPI bus, separate from TFT):");
  Serial.printf("  SCK=%d MOSI=%d MISO=%d CS=%d IRQ=%d\n",
                ELEGOO28_TOUCH_SCK, ELEGOO28_TOUCH_MOSI, ELEGOO28_TOUCH_MISO,
                ELEGOO28_TOUCH_CS, ELEGOO28_TOUCH_IRQ);
#if ELEGOO28_TOUCH_IRQ >= 0
  pinMode(ELEGOO28_TOUCH_IRQ, INPUT);
  Serial.printf("  IRQ level (1=idle): %d\n", digitalRead(ELEGOO28_TOUCH_IRQ));
#endif
}

static bool runCalibrationWizard(uint16_t cal[8]) {
  if (!display.touch()) {
    Serial.println("touch: driver not attached — skipping calibration");
    return false;
  }

  display.fillScreen(TFT_BLACK);
  display.setTextDatum(textdatum_t::middle_center);
  display.setTextColor(TFT_WHITE);
  display.drawString("Tap each corner target", display.width() >> 1, display.height() >> 1);
  display.setTextDatum(textdatum_t::top_left);

  const int dim = display.width() > display.height() ? display.width() : display.height();
  display.calibrateTouch(cal, TFT_WHITE, TFT_RED, (uint8_t)(dim >> 3));
  display.setTouchCalibrate(cal);
  elegoo28_touch_cal_save(cal, ELEGOO28_DISPLAY_ROTATION);
  elegoo28_touch_cal_print_serial(cal);
  Serial.println("touch: calibration saved to NVS");
  return true;
}

/** @return true if the interactive calibration wizard ran this boot. */
static bool setupTouchCalibration(void) {
  if (!display.touch()) {
    Serial.println("touch: driver not attached");
    return false;
  }

#if ELEGOO28_TOUCH_FORCE_CALIBRATE
  Serial.println("touch: FORCE_CALIBRATE — clearing NVS and re-running wizard");
  elegoo28_touch_cal_clear();
#endif

  uint16_t cal[8] = {0};
  const bool loaded = elegoo28_touch_cal_load(cal, ELEGOO28_DISPLAY_ROTATION);
  bool ranWizard = false;

#if ELEGOO28_TOUCH_REQUIRE_CALIBRATION
  if (!loaded || ELEGOO28_TOUCH_FORCE_CALIBRATE) {
    if (!runCalibrationWizard(cal)) {
      return false;
    }
    ranWizard = true;
  } else {
    display.setTouchCalibrate(cal);
    Serial.println("touch: loaded calibration from NVS");
    elegoo28_touch_cal_print_serial(cal);
  }
#else
  if (loaded) {
    display.setTouchCalibrate(cal);
    Serial.println("touch: loaded calibration from NVS");
    elegoo28_touch_cal_print_serial(cal);
  } else {
    Serial.println("touch: no NVS cal — using default range (set REQUIRE_CALIBRATION=1)");
  }
#endif

  return ranWizard;
}

static bool waitForTouchRelease(void) {
  uint32_t idleSince = 0;
  while (true) {
    int32_t tx = -1;
    int32_t ty = -1;
    if (!display.getTouch(&tx, &ty)) {
      if (idleSince == 0) {
        idleSince = millis();
      } else if (millis() - idleSince >= 120) {
        return true;
      }
    } else {
      idleSince = 0;
    }
    delay(10);
  }
}

static void runCornerValidation(void) {
  if (!display.touch()) {
    return;
  }

  const int32_t w = display.width();
  const int32_t h = display.height();
  const int32_t m = CORNER_MARGIN;

  CornerTarget targets[] = {
      {"TL", m, m},
      {"TR", w - 1 - m, m},
      {"BR", w - 1 - m, h - 1 - m},
      {"BL", m, h - 1 - m},
      {"C", w / 2, h / 2},
  };
  const size_t targetCount = sizeof(targets) / sizeof(targets[0]);

  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setCursor(4, 4);
  display.println("Corner test — tap each red target");
  for (size_t i = 0; i < targetCount; i++) {
    drawCornerTarget(targets[i], i == 0);
  }

  Serial.println("=== corner validation ===");
  bool allPass = true;

  for (size_t i = 0; i < targetCount; i++) {
    for (size_t j = 0; j < targetCount; j++) {
      drawCornerTarget(targets[j], j == i);
    }

    Serial.printf("tap target %s at (%ld, %ld)\n", targets[i].label, (long)targets[i].x,
                  (long)targets[i].y);

    bool logged = false;
    while (!logged) {
      int32_t tx = -1;
      int32_t ty = -1;
      if (display.getTouch(&tx, &ty)) {
        const int32_t dx = tx - targets[i].x;
        const int32_t dy = ty - targets[i].y;
        const bool pass =
            (abs(dx) <= ELEGOO28_TOUCH_CORNER_TOLERANCE) && (abs(dy) <= ELEGOO28_TOUCH_CORNER_TOLERANCE);

        Serial.printf("touch: x=%ld y=%ld\n", (long)tx, (long)ty);
        Serial.printf("corner %s error dx=%ld dy=%ld %s\n", targets[i].label, (long)dx, (long)dy,
                      pass ? "OK" : "FAIL");

        drawTouchMarker(tx, ty, pass ? TFT_GREEN : TFT_ORANGE);
        if (!pass) {
          allPass = false;
        }
        logged = true;
        waitForTouchRelease();
      }
      delay(10);
    }
  }

  display.setTextColor(allPass ? TFT_GREEN : TFT_ORANGE);
  display.setCursor(4, h - 14);
  display.printf(allPass ? "Corner test PASS" : "Corner test FAIL — see Serial");

  Serial.printf("corner validation: %s\n", allPass ? "PASS" : "FAIL");
  Serial.println("=== end corner validation ===");
  delay(2000);
}

#if BIRD_ALERT_WIFI_ENABLED

static void home_invalidate(uint8_t mask) {
  g_homeDirtyMask |= mask;
}

static void home_refresh_if_dirty(void) {
  if (g_homeDirtyMask == BIRD_ALERT_UI_DIRTY_NONE) {
    return;
  }
  const uint8_t mask = g_homeDirtyMask;
  g_homeDirtyMask = BIRD_ALERT_UI_DIRTY_NONE;
  bird_alert_ui_refresh(display, g_homeView, mask);
}

static void home_on_wifi_link_changed(void) {
  const BirdAlertWifiStatus wifi = bird_alert_wifi_status();
  if (!bird_alert_wifi_status_ui_changed(g_homeView.wifi, wifi)) {
    return;
  }
  g_homeView.wifi = wifi;
#if BIRD_ALERT_MQTT_ENABLED
  g_homeView.mqttConnected = bird_alert_mqtt_connected();
#endif
  home_invalidate(BIRD_ALERT_UI_DIRTY_HEADER | BIRD_ALERT_UI_DIRTY_MAIN);
}

#if BIRD_ALERT_MQTT_ENABLED
static void home_on_mqtt_connection_changed(bool connected) {
  if (g_homeView.mqttConnected == connected) {
    return;
  }
  g_homeView.mqttConnected = connected;
  home_invalidate(BIRD_ALERT_UI_DIRTY_HEADER | BIRD_ALERT_UI_DIRTY_MAIN);
}
#endif

static void home_process_link_events(void) {
  if (bird_alert_wifi_take_event()) {
    home_on_wifi_link_changed();
  }
#if BIRD_ALERT_MQTT_ENABLED
  bool mqttConnected = false;
  if (bird_alert_mqtt_take_connection_event(&mqttConnected)) {
    home_on_mqtt_connection_changed(mqttConnected);
  }
#endif
}

#if BIRD_ALERT_MQTT_ENABLED
static void clearActiveAlert(void) {
  g_homeView.mode = BIRD_ALERT_UI_IDLE;
  g_homeView.activeAlertId[0] = '\0';
  g_homeView.activeFrom[0] = '\0';
  g_homeView.activeAlertTs = 0;
  g_homeView.ackFrom[0] = '\0';
  g_ackedAtMs = 0;
  home_invalidate(BIRD_ALERT_UI_DIRTY_MAIN);
}

static void showAcknowledged(const char *from) {
  strncpy(g_homeView.ackFrom, from, sizeof(g_homeView.ackFrom) - 1);
  g_homeView.ackFrom[sizeof(g_homeView.ackFrom) - 1] = '\0';
  g_homeView.mode = BIRD_ALERT_UI_ACKED;
  g_ackedAtMs = millis();
  home_invalidate(BIRD_ALERT_UI_DIRTY_MAIN);
}

static void setShowingAlert(const char *alertId, const char *from, uint32_t ts) {
  strncpy(g_homeView.activeAlertId, alertId, sizeof(g_homeView.activeAlertId) - 1);
  g_homeView.activeAlertId[sizeof(g_homeView.activeAlertId) - 1] = '\0';
  strncpy(g_homeView.activeFrom, from, sizeof(g_homeView.activeFrom) - 1);
  g_homeView.activeFrom[sizeof(g_homeView.activeFrom) - 1] = '\0';
  g_homeView.activeAlertTs = ts;
  g_homeView.mode = BIRD_ALERT_UI_SHOWING;
  home_invalidate(BIRD_ALERT_UI_DIRTY_MAIN);
}

static void handleMqttEvent(const BirdAlertMqttEvent &event) {
  const char *selfId = bird_alert_device_id();

  if (strcmp(event.type, "raise") == 0) {
    if (strcmp(event.from, selfId) == 0) {
      return;
    }

    if (g_homeView.mode == BIRD_ALERT_UI_SHOWING && event.ts < g_homeView.activeAlertTs) {
      return;
    }

    setShowingAlert(event.alert_id, event.from, event.ts);
    return;
  }

  if (strcmp(event.type, "ack") == 0) {
    if (g_homeView.activeAlertId[0] == '\0' ||
        strcmp(event.alert_id, g_homeView.activeAlertId) != 0) {
      return;
    }

    if (g_homeView.mode == BIRD_ALERT_UI_SENT) {
      showAcknowledged(event.from);
      return;
    }

    if (g_homeView.mode == BIRD_ALERT_UI_SHOWING) {
      clearActiveAlert();
    }
  }
}

static void pollMqttEvents(void) {
  BirdAlertMqttEvent event;
  while (bird_alert_mqtt_poll_event(&event)) {
    handleMqttEvent(event);
  }
}

static void raiseLocalAlert(void) {
  const uint32_t ts = millis();
  char alertId[BIRD_ALERT_MQTT_ALERT_ID_MAX] = {0};
  bird_alert_mqtt_format_alert_id(alertId, sizeof(alertId), ts);

  if (!bird_alert_mqtt_publish_raise(alertId, ts)) {
    Serial.println("alert: publish raise failed");
    return;
  }

  strncpy(g_homeView.activeAlertId, alertId, sizeof(g_homeView.activeAlertId) - 1);
  g_homeView.activeAlertId[sizeof(g_homeView.activeAlertId) - 1] = '\0';
  strncpy(g_homeView.activeFrom, bird_alert_device_id(), sizeof(g_homeView.activeFrom) - 1);
  g_homeView.activeFrom[sizeof(g_homeView.activeFrom) - 1] = '\0';
  g_homeView.activeAlertTs = ts;
  g_homeView.mode = BIRD_ALERT_UI_SENT;
  home_invalidate(BIRD_ALERT_UI_DIRTY_MAIN);
}

static void acknowledgeActiveAlert(void) {
  if (g_homeView.activeAlertId[0] == '\0') {
    return;
  }

  const uint32_t ts = millis();
  if (bird_alert_mqtt_publish_ack(g_homeView.activeAlertId, ts)) {
    clearActiveAlert();
  } else {
    Serial.println("alert: publish ack failed");
  }
}

static void setupMqtt(void) {
  bird_alert_device_id_init();
  bird_alert_nickname_load();

  if (!bird_alert_mqtt_load(&g_mqttConfig) || g_mqttConfig.host[0] == '\0') {
    Serial.println("mqtt: no broker configured — opening setup UI");
    mqtt_setup_ui_run(display, &g_mqttConfig);
  } else if (!bird_alert_mqtt_begin(&g_mqttConfig)) {
    Serial.println("mqtt: connect failed — opening setup UI");
    mqtt_setup_ui_run(display, &g_mqttConfig);
  }

  g_homeView = {};
  g_homeView.mode = BIRD_ALERT_UI_IDLE;
  g_homeView.wifi = bird_alert_wifi_status();
  g_homeView.mqttConnected = bird_alert_mqtt_connected();
  home_invalidate(BIRD_ALERT_UI_DIRTY_FULL);
  home_refresh_if_dirty();
}
#endif

static void setupWifi(void) {
  bird_alert_wifi_events_init();

  char ssid[BIRD_ALERT_WIFI_SSID_MAX + 1] = {0};
  char pass[BIRD_ALERT_WIFI_PASS_MAX + 1] = {0};

  if (bird_alert_wifi_load(ssid, sizeof(ssid), pass, sizeof(pass)) &&
      bird_alert_wifi_connect(ssid, pass, 15000)) {
    Serial.println("wifi: auto-connected from NVS");
  } else {
    Serial.println("wifi: no saved creds or connect failed — opening setup UI");
    wifi_setup_ui_run(display);
  }

#if BIRD_ALERT_MQTT_ENABLED
  setupMqtt();
#else
  bird_alert_device_id_init();
  bird_alert_nickname_load();
  g_homeView = {};
  g_homeView.mode = BIRD_ALERT_UI_IDLE;
  g_homeView.wifi = bird_alert_wifi_status();
  g_homeView.mqttConnected = false;
  home_invalidate(BIRD_ALERT_UI_DIRTY_FULL);
  home_refresh_if_dirty();
#endif
}
#else
static void drawBringupScreen(void) {
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_GREEN);
  display.setTextSize(2);
  display.setCursor(8, 24);
  display.println("Bird Alert");
  display.setTextSize(1);
  display.setTextColor(TFT_WHITE);
  display.println(ELEGOO28_DISPLAY_NAME);
  display.println("LovyanGFX + touch");
  display.setTextColor(TFT_CYAN);
  display.println("Tap to test touch");
  display.drawRect(2, 2, display.width() - 4, display.height() - 4, TFT_CYAN);
}
#endif

void setup() {
  Serial.begin(115200);
  delay(2500);

  Serial.println();
  Serial.println("=== display_bringup (LovyanGFX) ===");
  Serial.println(ELEGOO28_DISPLAY_NAME);
  Serial.flush();

  elegoo28_display_backlight_on();
  logTouchPinConfig();

  display.init();
  display.setRotation(ELEGOO28_DISPLAY_ROTATION);
  display.setTextSize(1);

  Serial.printf("display: %dx%d rotation=%d\n", display.width(), display.height(), display.getRotation());
  Serial.printf("touch driver attached: %s\n", display.touch() ? "yes" : "no");
  Serial.println("(attached != aligned — run calibration wizard if coords are wrong)");
  Serial.flush();

  const bool calibratedThisBoot = setupTouchCalibration();

#if ELEGOO28_TOUCH_RUN_CORNER_TEST
  if (calibratedThisBoot) {
    runCornerValidation();
  }
#endif

#if BIRD_ALERT_WIFI_ENABLED
  setupWifi();
  Serial.println("Ready — home screen (SPOT A BIRD / Settings)");
#else
  drawBringupScreen();
  Serial.println("Ready — tap screen to verify");
#endif
  Serial.flush();
}

void loop() {
  const uint32_t now = millis();

#if BIRD_ALERT_WIFI_ENABLED
#if BIRD_ALERT_MQTT_ENABLED
  bird_alert_mqtt_loop();
  pollMqttEvents();
  home_process_link_events();

  if (g_homeView.mode == BIRD_ALERT_UI_ACKED && g_ackedAtMs != 0 &&
      now - g_ackedAtMs >= BIRD_ALERT_UI_ACKED_MS) {
    clearActiveAlert();
  }

  static uint32_t lastPulse;
  if (g_homeView.mode == BIRD_ALERT_UI_SENT && now - lastPulse >= 500) {
    lastPulse = now;
    g_homeView.pulsePhase = (g_homeView.pulsePhase + 1) % 3;
    home_invalidate(BIRD_ALERT_UI_DIRTY_PULSE);
  }

  int32_t tx = -1;
  int32_t ty = -1;
  if (display.getTouch(&tx, &ty)) {
    const int32_t w = display.width();
    const int32_t h = display.height();

    if (g_homeView.mode == BIRD_ALERT_UI_SHOWING && bird_alert_ui_hit_ack_button(tx, ty, w, h)) {
      waitForTouchRelease();
      acknowledgeActiveAlert();
    } else if (bird_alert_ui_can_alert(g_homeView) && bird_alert_ui_hit_primary_button(tx, ty, w, h)) {
      waitForTouchRelease();
      raiseLocalAlert();
    } else if (bird_alert_ui_hit_settings_button(tx, ty, w, h)) {
      waitForTouchRelease();
      bird_alert_settings_ui_run(display, &g_mqttConfig);
      g_homeView.wifi = bird_alert_wifi_status();
      g_homeView.mqttConnected = bird_alert_mqtt_connected();
      home_invalidate(BIRD_ALERT_UI_DIRTY_FULL);
    }
  }

  home_refresh_if_dirty();
#else
  home_process_link_events();

  int32_t tx = -1;
  int32_t ty = -1;
  if (display.getTouch(&tx, &ty)) {
    const int32_t w = display.width();
    const int32_t h = display.height();
    if (bird_alert_ui_hit_settings_button(tx, ty, w, h)) {
      waitForTouchRelease();
      wifi_setup_ui_run(display);
      g_homeView.wifi = bird_alert_wifi_status();
      home_invalidate(BIRD_ALERT_UI_DIRTY_FULL);
    }
  }

  home_refresh_if_dirty();
#endif
#else
  lgfx::touch_point_t raw;
  const uint8_t rawCount = display.getTouchRaw(&raw, 1);

#if ELEGOO28_TOUCH_DEBUG_RAW
  static uint32_t lastRawLog;
  static bool lastPressed;
  const bool pressed = (rawCount > 0);
  if (pressed != lastPressed || (pressed && now - lastRawLog >= 200)) {
    lastPressed = pressed;
    lastRawLog = now;
    if (pressed) {
      Serial.printf("raw: x=%d y=%d z=%d\n", raw.x, raw.y, raw.size);
    }
  }
#endif

  int32_t tx = -1;
  int32_t ty = -1;
  if (display.getTouch(&tx, &ty)) {
    Serial.printf("touch: x=%ld y=%ld\n", (long)tx, (long)ty);
    Serial.flush();

    drawTouchMarker(tx, ty, TFT_GREEN);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setCursor(4, 4);
    display.printf("x=%4ld y=%4ld   ", (long)tx, (long)ty);
  }
#endif

  static uint32_t lastHeartbeat;
  if (now - lastHeartbeat >= 5000) {
    lastHeartbeat = now;
#if ELEGOO28_TOUCH_IRQ >= 0
    Serial.printf("[heartbeat] ms=%lu irq=%d\n", now, digitalRead(ELEGOO28_TOUCH_IRQ));
#else
    Serial.printf("[heartbeat] ms=%lu\n", now);
#endif
    Serial.flush();
  }
}
