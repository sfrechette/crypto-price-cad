#include "api_client.h"
#include "secrets.h"
#include <time.h>

APIClient::APIClient() {
  lastError = "";
}

bool APIClient::connectWiFi(const char* ssid, const char* password, unsigned long timeout) {
  Serial.printf("Attempting to connect to WiFi: %s\n", ssid);
  
  // Disconnect any existing connection
  WiFi.disconnect(true);
  delay(1000);
  
  // Set WiFi mode
  WiFi.mode(WIFI_STA);
  delay(100);
  
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
    delay(500);
    Serial.print(".");
    
    // Print WiFi status for debugging
    wl_status_t status = WiFi.status();
    if (millis() - startTime > 10000) { // After 10 seconds, show status
      Serial.printf("\nWiFi Status: %d ", status);
      switch(status) {
        case WL_IDLE_STATUS: Serial.print("(IDLE)"); break;
        case WL_NO_SSID_AVAIL: Serial.print("(NO_SSID)"); break;
        case WL_SCAN_COMPLETED: Serial.print("(SCAN_COMPLETED)"); break;
        case WL_CONNECTED: Serial.print("(CONNECTED)"); break;
        case WL_CONNECT_FAILED: Serial.print("(CONNECT_FAILED)"); break;
        case WL_CONNECTION_LOST: Serial.print("(CONNECTION_LOST)"); break;
        case WL_DISCONNECTED: Serial.print("(DISCONNECTED)"); break;
        default: Serial.print("(UNKNOWN)"); break;
      }
      Serial.println();
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected successfully!");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal strength: %d dBm\n", WiFi.RSSI());
    return true;
  } else {
    wl_status_t finalStatus = WiFi.status();
    Serial.printf("\nWiFi connection failed. Final status: %d\n", finalStatus);
    
    String errorMsg = "WiFi connection failed: ";
    switch(finalStatus) {
      case WL_NO_SSID_AVAIL:
        errorMsg += "Network not found";
        break;
      case WL_CONNECT_FAILED:
        errorMsg += "Wrong password or connection failed";
        break;
      case WL_CONNECTION_LOST:
        errorMsg += "Connection lost";
        break;
      case WL_DISCONNECTED:
        errorMsg += "Disconnected";
        break;
      default:
        errorMsg += "Timeout or unknown error";
        break;
    }
    
    setError(errorMsg.c_str());
    return false;
  }
}

bool APIClient::isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool APIClient::fetchCryptoData(CryptoData cryptos[], int count) {
  if (!isWiFiConnected()) {
    setError("WiFi not connected");
    return false;
  }
  
  client.setInsecure(); // Skip SSL certificate verification
  http.begin(client, API_ENDPOINT);
  http.setTimeout(15000); // 15 second timeout
  http.addHeader("Accept", "application/json");
  
  Serial.println("Making API request to CoinMarketCap...");
  Serial.println(API_ENDPOINT);
  
  int httpCode = http.GET();
  
  Serial.printf("HTTP Response Code: %d\n", httpCode);
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    
    return parseJsonResponse(payload, cryptos, count);
  } else if (httpCode > 0) {
    // Got a response but not OK
    String errorPayload = http.getString();
    http.end();
    
    Serial.printf("HTTP Error Response: %s\n", errorPayload.c_str());
    
    // Check for common API errors
    if (httpCode == 401) {
      setError("API Key invalid or expired");
    } else if (httpCode == 403) {
      setError("API access forbidden - check your plan");  
    } else if (httpCode == 429) {
      setError("API rate limit exceeded");
    } else {
      setError(("HTTP error " + String(httpCode) + ": " + errorPayload).c_str());
    }
    return false;
  } else {
    // Connection error
    http.end();
    setError(("Connection failed: " + String(httpCode)).c_str());
    return false;
  }
}

bool APIClient::parseJsonResponse(const String& payload, CryptoData cryptos[], int count) {
  Serial.println("=== API Response Debug ===");
  Serial.printf("Payload length: %d\n", payload.length());
  Serial.println("First 500 characters of response:");
  Serial.println(payload.substring(0, 500));
  Serial.println("=========================");
  
  // Use larger buffer for CoinMarketCap API response (32KB for multi-crypto JSON)
  DynamicJsonDocument doc(32768);
  
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.printf("JSON parsing error: %s\n", error.c_str());
    setError(("JSON parsing failed: " + String(error.c_str())).c_str());
    return false;
  }
  
  // Check if response has the expected structure
  if (!doc.containsKey("data")) {
    Serial.println("Response missing 'data' key");
    setError("API response missing 'data' section");
    return false;
  }
  
  // Parse each cryptocurrency
  for (int i = 0; i < count; i++) {
    const char* symbol = cryptos[i].symbol;
    Serial.printf("Parsing %s...\n", symbol);
    
    // Check if symbol exists in data
    if (!doc["data"].containsKey(symbol)) {
      Serial.printf("Missing data for %s\n", symbol);
      setError(("Missing data for " + String(symbol)).c_str());
      return false;
    }
    
    // Check if it's an array with at least one element
    if (!doc["data"][symbol].is<JsonArray>() || doc["data"][symbol].size() == 0) {
      Serial.printf("Invalid data structure for %s\n", symbol);
      setError(("Invalid data structure for " + String(symbol)).c_str());
      return false;
    }
    
    // Check for price data
    if (!doc["data"][symbol][0]["quote"]["CAD"]["price"].is<float>()) {
      Serial.printf("Missing price data for %s\n", symbol);
      setError(("Missing price data for " + String(symbol)).c_str());
      return false;
    }
    
    // Extract the new price and update tracking
    float newPrice = doc["data"][symbol][0]["quote"]["CAD"]["price"];
    
    // Track price movement (only if not first update)
    if (!cryptos[i].firstUpdate && newPrice != cryptos[i].price) {
      cryptos[i].priceIncreased = (newPrice > cryptos[i].price);
      cryptos[i].previousPrice = cryptos[i].price;
    }
    
    // Update price and timestamp
    cryptos[i].price = newPrice;
    cryptos[i].lastUpdated = doc["data"][symbol][0]["quote"]["CAD"]["last_updated"];
    cryptos[i].firstUpdate = false;
    
    Serial.printf("%s price: %.2f CAD\n", symbol, cryptos[i].price);
  }
  
  Serial.println("JSON parsing successful!");
  return true;
}

bool APIClient::fetchStockData(AssetData& stock) {
  if (!isWiFiConnected()) {
    setError("WiFi not connected");
    return false;
  }
  
  client.setInsecure(); // Skip SSL certificate verification
  http.begin(client, STOCK_ENDPOINT);
  http.setTimeout(15000); // 15 second timeout
  http.addHeader("Accept", "application/json");
  
  Serial.println("Making API request to Financial Modeling Prep...");
  Serial.println(STOCK_ENDPOINT);
  
  int httpCode = http.GET();
  Serial.printf("HTTP Response Code: %d\n", httpCode);
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    http.end();
    return parseStockJsonResponse(payload, stock);
  } else if (httpCode > 0) {
    String errorPayload = http.getString();
    http.end();
    Serial.printf("HTTP Error Response: %s\n", errorPayload.c_str());
    
    if (httpCode == 401) {
      setError("Stock API Key invalid or expired");
    } else if (httpCode == 403) {
      setError("Stock API access forbidden - check your plan");
    } else if (httpCode == 429) {
      setError("Stock API rate limit exceeded");
    } else {
      setError(("HTTP error " + String(httpCode) + ": " + errorPayload).c_str());
    }
    return false;
  } else {
    http.end();
    setError(("Stock API connection failed: " + String(httpCode)).c_str());
    return false;
  }
}

bool APIClient::parseStockJsonResponse(const String& payload, AssetData& stock) {
  Serial.println("=== Stock API Response Debug ===");
  Serial.printf("Payload length: %d\n", payload.length());
  Serial.println("First 200 characters of response:");
  Serial.println(payload.substring(0, 200));
  Serial.println("================================");
  
  DynamicJsonDocument doc(8192); // 8KB buffer sufficient for single stock response
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.printf("Stock JSON parsing error: %s\n", error.c_str());
    setError(("Stock JSON parsing failed: " + String(error.c_str())).c_str());
    return false;
  }
  
  // FMP stable API returns an array with one object for single stock quote
  if (!doc.is<JsonArray>() || doc.size() == 0) {
    Serial.println("Stock response is not an array or is empty");
    setError("Invalid stock API response structure");
    return false;
  }
  
  JsonObject stockObj = doc[0];
  
  if (!stockObj.containsKey("price")) {
    Serial.println("Missing 'price' field in stock response");
    setError("Missing stock price data");
    return false;
  }
  
  // Extract new stock price and track movement
  float newPrice = stockObj["price"];
  
  // Track price movement (only if not first update)
  if (!stock.firstUpdate && newPrice != stock.price) {
    stock.priceIncreased = (newPrice > stock.price);
    stock.previousPrice = stock.price;
  }
  
  stock.price = newPrice;
  stock.firstUpdate = false;
  
  // Extract timestamp and format it like crypto (ISO 8601 format)
  // NOTE: Static buffer avoids heap allocation and persists beyond function scope
  static char timestampBuffer[32];
  
  if (stockObj.containsKey("timestamp")) {
    // FMP provides Unix timestamp, convert to ISO 8601 format like crypto
    unsigned long timestamp = stockObj["timestamp"];
    
    // Convert Unix timestamp to readable format
    time_t rawtime = timestamp;
    struct tm * timeinfo = gmtime(&rawtime);
    strftime(timestampBuffer, sizeof(timestampBuffer), "%Y-%m-%dT%H:%M:%S.000Z", timeinfo);
    stock.lastUpdated = timestampBuffer;
    
    Serial.printf("Converted timestamp %lu to: %s\n", timestamp, timestampBuffer);
  } else {
    // Fallback if no timestamp field
    stock.lastUpdated = "Just now";
    Serial.println("No timestamp field found, using 'Just now'");
  }
  
  Serial.printf("%s price extracted: %.2f USD\n", stock.symbol, stock.price);
  Serial.println("Stock JSON parsing successful!");
  
  return true;
}

const char* APIClient::getLastError() {
  return lastError.c_str();
}

void APIClient::scanNetworks() {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("Found %d networks:\n", n);
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (%d dBm) %s\n", 
                   i + 1, 
                   WiFi.SSID(i).c_str(), 
                   WiFi.RSSI(i),
                   (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");
    }
  }
  WiFi.scanDelete();
}

void APIClient::setError(const char* error) {
  lastError = String(error);
  Serial.printf("API Error: %s\n", error);
}
