#include "mqtt_client.h"
#include "secrets.h"
#include <ArduinoJson.h>

MQTTClient::MQTTClient() : client(wifiClient) {
  lastReconnectAttempt = 0;
  mqttBroker = nullptr;
  mqttPort = 1883;
  mqttUser = "";
  mqttPassword = "";
}

bool MQTTClient::begin(const char* broker, int port, const char* user, const char* password) {
  mqttBroker = broker;
  mqttPort = port;
  mqttUser = user;
  mqttPassword = password;
  
  client.setServer(broker, port);
  
  // Set buffer size for larger discovery messages (must be > 600 for discovery JSON)
  client.setBufferSize(1024);
  
  Serial.printf("MQTT: Connecting to broker %s:%d\n", broker, port);
  Serial.printf("MQTT: Credentials - user='%s', pass length=%d\n", 
                user ? user : "NULL", 
                password ? strlen(password) : 0);
  
  return reconnect();
}

void MQTTClient::loop() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}

bool MQTTClient::isConnected() {
  return client.connected();
}

bool MQTTClient::reconnect() {
  if (client.connected()) {
    return true;
  }
  
  Serial.println("MQTT: Attempting connection...");
  
  // Build Last Will and Testament topic
  String statusTopic = buildTopic("/status");
  
  // Use credentials from secrets.h defines directly
  Serial.printf("MQTT: User='%s', Pass length=%d\n", MQTT_USER, strlen(MQTT_PASSWORD));
  
  bool connected = false;
  
  Serial.println("MQTT: Connecting with authentication...");
  connected = client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, 
                             statusTopic.c_str(), 0, true, "offline");
  
  if (connected) {
    Serial.println("MQTT: Connected successfully!");
    // Publish online status
    publishAvailability(true);
    return true;
  } else {
    Serial.printf("MQTT: Connection failed, state=%d\n", client.state());
    // Decode error state
    switch(client.state()) {
      case -4: Serial.println("  -> MQTT_CONNECTION_TIMEOUT"); break;
      case -3: Serial.println("  -> MQTT_CONNECTION_LOST"); break;
      case -2: Serial.println("  -> MQTT_CONNECT_FAILED"); break;
      case -1: Serial.println("  -> MQTT_DISCONNECTED"); break;
      case 1: Serial.println("  -> MQTT_CONNECT_BAD_PROTOCOL"); break;
      case 2: Serial.println("  -> MQTT_CONNECT_BAD_CLIENT_ID"); break;
      case 3: Serial.println("  -> MQTT_CONNECT_UNAVAILABLE"); break;
      case 4: Serial.println("  -> MQTT_CONNECT_BAD_CREDENTIALS"); break;
      case 5: Serial.println("  -> MQTT_CONNECT_UNAUTHORIZED"); break;
    }
    return false;
  }
}

void MQTTClient::publishAvailability(bool online) {
  String topic = buildTopic("/status");
  const char* payload = online ? "online" : "offline";
  client.publish(topic.c_str(), payload, true); // Retained
  Serial.printf("MQTT: Published availability: %s\n", payload);
}

void MQTTClient::publishDiscoveryConfigs(AssetData assets[], int count) {
  if (!client.connected()) {
    Serial.println("MQTT: Cannot publish discovery - not connected");
    return;
  }
  
  Serial.println("MQTT: Publishing Home Assistant discovery configs...");
  
  for (int i = 0; i < count; i++) {
    publishAssetDiscovery(assets[i]);
    delay(100); // Small delay between messages to avoid overwhelming broker
  }
  
  Serial.println("MQTT: Discovery configs published!");
}

void MQTTClient::publishAssetDiscovery(const AssetData& asset) {
  // Build discovery topic: homeassistant/sensor/m5crypto_btc/config
  String symbol = String(asset.symbol);
  symbol.toLowerCase();
  
  String discoveryTopic = "homeassistant/sensor/m5crypto_" + symbol + "/config";
  
  // Build state topic
  String stateTopic = buildTopic("/" + symbol + "/state");
  String availabilityTopic = buildTopic("/status");
  
  // Create JSON discovery payload
  StaticJsonDocument<512> doc;
  
  // Basic sensor config
  doc["name"] = String(asset.name) + " Price";
  doc["unique_id"] = "m5crypto_" + symbol + "_price";
  doc["state_topic"] = stateTopic;
  doc["value_template"] = "{{ value_json.price }}";
  doc["unit_of_measurement"] = asset.currency;
  doc["icon"] = getIcon(asset.symbol);
  doc["state_class"] = "measurement";
  doc["availability_topic"] = availabilityTopic;
  
  // Device info (groups all sensors under one device in HA)
  JsonObject device = doc.createNestedObject("device");
  device["identifiers"][0] = "m5crypto_display";
  device["name"] = "Crypto Price Display";
  device["model"] = "M5StickC Plus2";
  device["manufacturer"] = "M5Stack";
  device["sw_version"] = "2.2";
  
  // Additional attributes (trend, timestamp)
  doc["json_attributes_topic"] = stateTopic;
  doc["json_attributes_template"] = "{{ {'trend': value_json.trend, 'updated': value_json.updated} | tojson }}";
  
  // Serialize and publish
  String payload;
  serializeJson(doc, payload);
  
  bool success = client.publish(discoveryTopic.c_str(), payload.c_str(), true); // Retained
  Serial.printf("MQTT: Discovery %s -> %s\n", asset.symbol, success ? "OK" : "FAILED");
}

void MQTTClient::publishPrices(AssetData assets[], int count) {
  if (!client.connected()) {
    Serial.println("MQTT: Cannot publish prices - not connected");
    return;
  }
  
  Serial.println("MQTT: Publishing price updates...");
  
  for (int i = 0; i < count; i++) {
    publishAssetState(assets[i]);
  }
  
  Serial.println("MQTT: Price updates published!");
}

void MQTTClient::publishAssetState(const AssetData& asset) {
  String symbol = String(asset.symbol);
  symbol.toLowerCase();
  
  String topic = buildTopic("/" + symbol + "/state");
  
  // Create JSON state payload
  StaticJsonDocument<256> doc;
  
  // Round price appropriately based on value
  if (asset.price >= 100) {
    doc["price"] = round(asset.price * 100) / 100; // 2 decimal places
  } else if (asset.price >= 1) {
    doc["price"] = round(asset.price * 1000) / 1000; // 3 decimal places
  } else {
    doc["price"] = round(asset.price * 10000) / 10000; // 4 decimal places
  }
  
  // Determine trend
  if (asset.firstUpdate) {
    doc["trend"] = "unknown";
  } else if (asset.priceIncreased) {
    doc["trend"] = "up";
  } else {
    doc["trend"] = "down";
  }
  
  // Include last update timestamp
  doc["updated"] = asset.lastUpdated;
  
  // Serialize and publish
  String payload;
  serializeJson(doc, payload);
  
  bool success = client.publish(topic.c_str(), payload.c_str());
  Serial.printf("MQTT: %s $%.2f %s -> %s\n", 
                asset.symbol, asset.price, asset.currency, 
                success ? "OK" : "FAILED");
}

String MQTTClient::buildTopic(const char* suffix) {
  return String(MQTT_TOPIC_PREFIX) + suffix;
}

String MQTTClient::buildTopic(const String& suffix) {
  return String(MQTT_TOPIC_PREFIX) + suffix;
}

const char* MQTTClient::getIcon(const char* symbol) {
  if (strcmp(symbol, "BTC") == 0) return "mdi:bitcoin";
  if (strcmp(symbol, "ETH") == 0) return "mdi:ethereum";
  // Note: For the actual XRP logo, install Simple Icons via HACS and use "si:xrp"
  if (strcmp(symbol, "XRP") == 0) return "mdi:alpha-x-circle";
  if (strcmp(symbol, "MSFT") == 0) return "mdi:microsoft";
  return "mdi:cash";
}

