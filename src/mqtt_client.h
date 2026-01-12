#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "crypto_display.h"

class MQTTClient {
public:
  MQTTClient();
  
  // Initialize and connect to MQTT broker
  bool begin(const char* broker, int port, const char* user = "", const char* password = "");
  
  // Maintain connection (call in loop)
  void loop();
  
  // Check connection status
  bool isConnected();
  
  // Reconnect if disconnected
  bool reconnect();
  
  // Publish discovery configs to Home Assistant (call once at startup)
  void publishDiscoveryConfigs(AssetData assets[], int count);
  
  // Publish current prices for all assets
  void publishPrices(AssetData assets[], int count);
  
  // Publish device availability status
  void publishAvailability(bool online);

private:
  WiFiClient wifiClient;
  PubSubClient client;
  
  const char* mqttBroker;
  int mqttPort;
  const char* mqttUser;
  const char* mqttPassword;
  
  unsigned long lastReconnectAttempt;
  static constexpr unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds between attempts
  
  // Helper to build topic strings
  String buildTopic(const char* suffix);
  String buildTopic(const String& suffix);
  
  // Publish a single asset's discovery config
  void publishAssetDiscovery(const AssetData& asset);
  
  // Publish a single asset's state
  void publishAssetState(const AssetData& asset);
  
  // Get MDI icon for asset
  const char* getIcon(const char* symbol);
};

#endif // MQTT_CLIENT_H

