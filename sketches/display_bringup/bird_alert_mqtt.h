#pragma once

/**
 * MQTT client for Bird Alert — subscribe/publish on bird/events.
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <stdio.h>

#include "bird_alert_device_id.h"
#include "bird_alert_mqtt_prefs.h"

#define BIRD_ALERT_MQTT_TOPIC_EVENTS "bird/events"
#define BIRD_ALERT_MQTT_KEEPALIVE_SEC 60

struct BirdAlertMqttEvent {
  bool valid;
  char type[8];
  char alert_id[BIRD_ALERT_MQTT_ALERT_ID_MAX];
  char from[BIRD_ALERT_DEVICE_ID_MAX];
  uint32_t ts;
};

static WiFiClient g_bird_alert_mqtt_wifi_client;
static PubSubClient g_bird_alert_mqtt_client(g_bird_alert_mqtt_wifi_client);
static BirdAlertMqttConfig g_bird_alert_mqtt_config = {};
static bool g_bird_alert_mqtt_subscribed = false;
static uint32_t g_bird_alert_mqtt_last_reconnect_attempt = 0;
static const uint32_t BIRD_ALERT_MQTT_RECONNECT_INTERVAL_MS = 5000;

static BirdAlertMqttEvent g_bird_alert_mqtt_pending_event = {};
static bool g_bird_alert_mqtt_was_connected = false;
static volatile bool g_bird_alert_mqtt_connection_pending = false;
static volatile bool g_bird_alert_mqtt_connection_state = false;

static inline void bird_alert_mqtt_notify_connection(bool connected) {
  if (connected == g_bird_alert_mqtt_was_connected) {
    return;
  }
  g_bird_alert_mqtt_was_connected = connected;
  g_bird_alert_mqtt_connection_state = connected;
  g_bird_alert_mqtt_connection_pending = true;
}

static inline bool bird_alert_mqtt_take_connection_event(bool *connectedOut) {
  if (!g_bird_alert_mqtt_connection_pending) {
    return false;
  }
  g_bird_alert_mqtt_connection_pending = false;
  if (connectedOut != nullptr) {
    *connectedOut = g_bird_alert_mqtt_connection_state;
  }
  return true;
}

static inline void bird_alert_mqtt_on_message(char *topic, byte *payload, unsigned int length) {
  (void)topic;

  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, payload, length);
  if (err) {
    Serial.printf("mqtt: json parse error: %s\n", err.c_str());
    return;
  }

  const int version = doc["v"] | 0;
  if (version != 1) {
    Serial.println("mqtt: unsupported message version");
    return;
  }

  const char *type = doc["type"] | "";
  const char *alertId = doc["alert_id"] | "";
  const char *from = doc["from"] | "";
  const uint32_t ts = doc["ts"] | 0U;

  if (type[0] == '\0' || alertId[0] == '\0' || from[0] == '\0') {
    Serial.println("mqtt: incomplete event");
    return;
  }

  if (strcmp(type, "raise") != 0 && strcmp(type, "ack") != 0) {
    Serial.printf("mqtt: unknown type \"%s\"\n", type);
    return;
  }

  BirdAlertMqttEvent event = {};
  event.valid = true;
  strncpy(event.type, type, sizeof(event.type) - 1);
  strncpy(event.alert_id, alertId, sizeof(event.alert_id) - 1);
  strncpy(event.from, from, sizeof(event.from) - 1);
  event.ts = ts;

  g_bird_alert_mqtt_pending_event = event;

  Serial.printf("mqtt: event %s alert_id=%s from=%s ts=%lu\n", event.type, event.alert_id, event.from,
                (unsigned long)event.ts);
}

static inline bool bird_alert_mqtt_connect_once(void) {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  if (g_bird_alert_mqtt_config.host[0] == '\0') {
    return false;
  }

  g_bird_alert_mqtt_client.setServer(g_bird_alert_mqtt_config.host, g_bird_alert_mqtt_config.port);
  g_bird_alert_mqtt_client.setCallback(bird_alert_mqtt_on_message);
  g_bird_alert_mqtt_client.setBufferSize(256);

  Serial.printf("mqtt: connecting to %s:%u as %s...\n", g_bird_alert_mqtt_config.host,
                (unsigned)g_bird_alert_mqtt_config.port, bird_alert_mqtt_client_id());

  const bool ok = g_bird_alert_mqtt_client.connect(bird_alert_mqtt_client_id(), g_bird_alert_mqtt_config.user,
                                                   g_bird_alert_mqtt_config.pass);

  if (!ok) {
    Serial.printf("mqtt: connect failed (state=%d)\n", g_bird_alert_mqtt_client.state());
    g_bird_alert_mqtt_subscribed = false;
    return false;
  }

  Serial.println("mqtt: connected");

  if (!g_bird_alert_mqtt_client.subscribe(BIRD_ALERT_MQTT_TOPIC_EVENTS)) {
    Serial.println("mqtt: subscribe failed");
    g_bird_alert_mqtt_subscribed = false;
    return false;
  }

  g_bird_alert_mqtt_subscribed = true;
  Serial.printf("mqtt: subscribed to %s\n", BIRD_ALERT_MQTT_TOPIC_EVENTS);
  return true;
}

static inline bool bird_alert_mqtt_begin(const BirdAlertMqttConfig *cfg) {
  if (cfg != nullptr) {
    g_bird_alert_mqtt_config = *cfg;
  } else {
    bird_alert_mqtt_load(&g_bird_alert_mqtt_config);
  }

  bird_alert_device_id_init();
  g_bird_alert_mqtt_pending_event = {};
  g_bird_alert_mqtt_subscribed = false;
  g_bird_alert_mqtt_last_reconnect_attempt = 0;
  g_bird_alert_mqtt_was_connected = false;
  g_bird_alert_mqtt_connection_pending = false;

  const bool ok = bird_alert_mqtt_connect_once();
  g_bird_alert_mqtt_was_connected = g_bird_alert_mqtt_client.connected();
  return ok;
}

static inline bool bird_alert_mqtt_connected(void) {
  return g_bird_alert_mqtt_client.connected();
}

static inline const BirdAlertMqttConfig *bird_alert_mqtt_config(void) {
  return &g_bird_alert_mqtt_config;
}

static inline void bird_alert_mqtt_loop(void) {
  if (!g_bird_alert_mqtt_client.connected()) {
    g_bird_alert_mqtt_subscribed = false;
    const uint32_t now = millis();
    if (now - g_bird_alert_mqtt_last_reconnect_attempt >= BIRD_ALERT_MQTT_RECONNECT_INTERVAL_MS) {
      g_bird_alert_mqtt_last_reconnect_attempt = now;
      bird_alert_mqtt_connect_once();
    }
  } else {
    g_bird_alert_mqtt_client.loop();
  }

  bird_alert_mqtt_notify_connection(g_bird_alert_mqtt_client.connected());
}

static inline bool bird_alert_mqtt_poll_event(BirdAlertMqttEvent *out) {
  if (out == nullptr || !g_bird_alert_mqtt_pending_event.valid) {
    return false;
  }

  *out = g_bird_alert_mqtt_pending_event;
  g_bird_alert_mqtt_pending_event.valid = false;
  return true;
}

static inline bool bird_alert_mqtt_publish_event(const char *type, const char *alert_id, uint32_t ts) {
  if (!bird_alert_mqtt_connected() || type == nullptr || alert_id == nullptr) {
    return false;
  }

  JsonDocument doc;
  doc["v"] = 1;
  doc["type"] = type;
  doc["alert_id"] = alert_id;
  doc["from"] = bird_alert_device_id();
  doc["ts"] = ts;

  char payload[192];
  const size_t len = serializeJson(doc, payload, sizeof(payload));
  if (len == 0) {
    return false;
  }

  const bool ok = g_bird_alert_mqtt_client.publish(BIRD_ALERT_MQTT_TOPIC_EVENTS, payload, false);
  if (ok) {
    Serial.printf("mqtt: published %s alert_id=%s\n", type, alert_id);
  } else {
    Serial.println("mqtt: publish failed");
  }
  return ok;
}

static inline bool bird_alert_mqtt_publish_raise(const char *alert_id, uint32_t ts) {
  return bird_alert_mqtt_publish_event("raise", alert_id, ts);
}

static inline bool bird_alert_mqtt_publish_ack(const char *alert_id, uint32_t ts) {
  return bird_alert_mqtt_publish_event("ack", alert_id, ts);
}

static inline void bird_alert_mqtt_format_alert_id(char *out, size_t outLen, uint32_t ts) {
  if (out == nullptr || outLen == 0) {
    return;
  }
  snprintf(out, outLen, "%s-%lu", bird_alert_device_id(), (unsigned long)ts);
}

static inline void bird_alert_mqtt_disconnect(void) {
  g_bird_alert_mqtt_client.disconnect();
  g_bird_alert_mqtt_subscribed = false;
}

static inline void bird_alert_mqtt_reconnect_after_config_change(const BirdAlertMqttConfig *cfg) {
  bird_alert_mqtt_disconnect();
  if (cfg != nullptr) {
    g_bird_alert_mqtt_config = *cfg;
  }
  g_bird_alert_mqtt_last_reconnect_attempt = 0;
  g_bird_alert_mqtt_was_connected = false;
  g_bird_alert_mqtt_connection_pending = false;
  bird_alert_mqtt_connect_once();
  g_bird_alert_mqtt_was_connected = g_bird_alert_mqtt_client.connected();
}
