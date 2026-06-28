#pragma once

/**
 * Deferred WiFi events for home UI updates (do not draw from the event handler).
 */

#include <Arduino.h>
#include <WiFi.h>

static volatile bool g_bird_alert_wifi_event_pending = false;

static inline void bird_alert_wifi_events_init(void) {
  static bool registered = false;
  if (registered) {
    return;
  }
  registered = true;

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    (void)info;
    switch (event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        g_bird_alert_wifi_event_pending = true;
        break;
      default:
        break;
    }
  });
}

static inline bool bird_alert_wifi_take_event(void) {
  if (!g_bird_alert_wifi_event_pending) {
    return false;
  }
  g_bird_alert_wifi_event_pending = false;
  return true;
}
