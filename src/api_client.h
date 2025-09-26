#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "crypto_display.h"

class APIClient {
public:
  APIClient();
  
  // Initialize WiFi connection
  bool connectWiFi(const char* ssid, const char* password, unsigned long timeout = 20000);
  
  // Check if WiFi is connected
  bool isWiFiConnected();
  
  // Fetch cryptocurrency data from API
  bool fetchCryptoData(CryptoData cryptos[], int count);
  
  // Fetch stock data from Financial Modeling Prep API
  bool fetchStockData(AssetData& stock);
  
  // Get last error message
  const char* getLastError();
  
  // Scan for available WiFi networks (diagnostic)
  void scanNetworks();
  
private:
  String lastError;
  WiFiClientSecure client;
  HTTPClient http;
  
  // Helper functions
  bool parseJsonResponse(const String& payload, CryptoData cryptos[], int count);
  bool parseStockJsonResponse(const String& payload, AssetData& stock);
  void setError(const char* error);
};

#endif // API_CLIENT_H
