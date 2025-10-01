/* 
 * Cryptocurrency & Stock Price Display for M5StickC Plus2
 * Author: Stéphane Fréchette
 * Version: 2.1 (With MSFT Stock)
 * 
 * Features:
 * - Displays BTC, ETH, XRP prices in CAD + MSFT stock price in USD
 * - Updates every 5 minutes from CoinMarketCap API + Financial Modeling Prep API
 * - Unified display rotation: BTC → ETH → XRP → MSFT → (repeat)
 * - Modular, maintainable code structure
 * - Proper error handling and recovery
 * - Optimized performance and memory usage
 * - Works with M5StickC Plus2 (ESP32-PICO-V3-02) via M5Unified library
 */

#include <Arduino.h>
#include <M5Unified.h>
#include <time.h>
#include "config.h"
#include "crypto_display.h"
#include "api_client.h"
#include "secrets.h"

// Global objects
CryptoDisplay display;
APIClient apiClient;

// Asset data array (crypto + stocks) - Adjusted for proportional font widths
AssetData assets[] = {
  {"BTC", "Bitcoin", 0.0, "", 0, 0, 90, false, "CAD", 0.0, false, true},     // Crypto - "Bitcoin" adjusted for actual width
  {"ETH", "Ethereum", 0.0, "", 0, 0, 102, false, "CAD", 0.0, false, true},   // Crypto - "Ethereum" adjusted for actual width
  {"XRP", "XRP", 0.0, "", 0, 0, 42, false, "CAD", 0.0, false, true},         // Crypto - "XRP" adjusted for actual width  
  {"MSFT", "Microsoft", 0.0, "Market Closed", 0, 0, 120, true, "USD", 0.0, false, true}   // Stock - "Microsoft" = adjusted for actual width
};
const int assetCount = sizeof(assets) / sizeof(assets[0]);

// Timing variables
unsigned long lastApiUpdate = 0;
unsigned long lastDisplaySwitch = 0;
int currentAssetIndex = 0;
bool dataLoaded = false;

// Brightness control variables - M5Unified API (works on all M5 devices)
constexpr uint8_t BRIGHTNESS_LEVELS[] = {51, 102, 153, 204, 255}; // 5 levels: 20%, 40%, 60%, 80%, 100%
constexpr uint8_t BRIGHTNESS_LEVEL_COUNT = sizeof(BRIGHTNESS_LEVELS) / sizeof(BRIGHTNESS_LEVELS[0]);
constexpr uint8_t BRIGHTNESS_MAX = 255;
uint8_t currentBrightnessIndex = 0; // Start at lowest (20%)
unsigned long lastButtonPress = 0;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 200; // Debounce delay

// Function declarations
bool fetchAndUpdateData();
void cycleBrightness();
bool isMarketOpen();
void setupTime();

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Cryptocurrency Price Display v2.1 (M5StickC Plus2) ===");

  // Initialize M5StickC Plus2
  M5.begin();
  display.begin();
  
  // Set initial brightness - M5Unified API
  M5.Display.setBrightness(BRIGHTNESS_LEVELS[currentBrightnessIndex]);
  Serial.printf("Initial brightness set to: %d/255 (%d%%)\n", 
                BRIGHTNESS_LEVELS[currentBrightnessIndex], 
                (BRIGHTNESS_LEVELS[currentBrightnessIndex] * 100) / BRIGHTNESS_MAX);
  
  // Show WiFi connection status
  display.displayWiFiStatus("Connecting...");
  
  // Optional: Scan for available networks for diagnostics (comment out to speed up startup)
  // apiClient.scanNetworks();
  
  // Connect to WiFi with timeout
  if (!apiClient.connectWiFi(WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_TIMEOUT)) {
    display.displayError("WiFi connection failed");
    Serial.printf("WiFi Error: %s\n", apiClient.getLastError());
    Serial.println("Retrying in 10 seconds...");
    delay(10000);
    ESP.restart(); // Restart and try again
  }
  
  display.displayWiFiStatus("Connected! Loading data...");
  
  // Setup time synchronization with NTP
  setupTime();
  
  // Initial data fetch
  if (fetchAndUpdateData()) {
    dataLoaded = true;
    Serial.println("Initial data loaded successfully");
  } else {
    display.displayError("Failed to load initial data");
    delay(3000);
  }
  
  lastApiUpdate = millis();
  lastDisplaySwitch = millis();
}

void loop() {
  M5.update(); // Handle button presses
  
  unsigned long currentTime = millis();
  
  // Handle Button A press for brightness control
  if (M5.BtnA.wasPressed() && (currentTime - lastButtonPress > BUTTON_DEBOUNCE_MS)) {
    cycleBrightness();
    lastButtonPress = currentTime;
  }
  
  // Check if we need to update data from API
  if (currentTime - lastApiUpdate >= API_UPDATE_INTERVAL) {
    display.displayWiFiStatus("Updating prices...");
    
    if (fetchAndUpdateData()) {
      dataLoaded = true;
      Serial.println("Data updated successfully");
    } else {
      Serial.println("Failed to update data, using cached values");
      display.displayError("Update failed");
      delay(2000); // Show error briefly
    }
    
    lastApiUpdate = currentTime;
    lastDisplaySwitch = currentTime; // Reset display timer
  }
  
  // Display asset data if available
  if (dataLoaded) {
    // Switch to next asset every DISPLAY_DURATION milliseconds
    if (currentTime - lastDisplaySwitch >= DISPLAY_DURATION) {
      currentAssetIndex = (currentAssetIndex + 1) % assetCount;
      lastDisplaySwitch = currentTime;
    }
    
    display.displayAsset(assets[currentAssetIndex]);
  }
  
  // Small delay to prevent excessive CPU usage (50ms = responsive button presses)
  delay(50);
}

bool fetchAndUpdateData() {
  if (!apiClient.isWiFiConnected()) {
    Serial.println("WiFi disconnected, attempting reconnection...");
    if (!apiClient.connectWiFi(WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_TIMEOUT)) {
      return false;
    }
  }
  
  bool cryptoSuccess = false;
  bool stockSuccess = false;
  
  // Fetch cryptocurrency data (first 3 assets) - pass directly, no copying needed
  constexpr int cryptoCount = 3;
  if (apiClient.fetchCryptoData(assets, cryptoCount)) {
    Serial.println("Successfully fetched cryptocurrency data:");
    // Update price tracking for each crypto asset
    for (int i = 0; i < cryptoCount; i++) {
      Serial.printf("  %s: $%.2f %s", assets[i].symbol, assets[i].price, assets[i].currency);
      if (!assets[i].firstUpdate) {
        Serial.printf(" (%s)", assets[i].priceIncreased ? "UP" : "DOWN");
      }
      Serial.println();
    }
    cryptoSuccess = true;
  } else {
    Serial.printf("Failed to fetch crypto data: %s\n", apiClient.getLastError());
  }
  
  // Fetch stock data (MSFT - index 3) - price tracking handled in API client
  if (apiClient.fetchStockData(assets[3])) {
    // Update status based on market hours
    if (isMarketOpen()) {
      // Market is open - keep the API timestamp
      Serial.printf("Successfully fetched stock data (market open): %s: $%.2f %s", 
                    assets[3].symbol, assets[3].price, assets[3].currency);
    } else {
      // Market is closed - show last price but with "Market Closed" status
      assets[3].lastUpdated = "Market Closed";
      Serial.printf("Successfully fetched stock data (market closed): %s: $%.2f %s", 
                    assets[3].symbol, assets[3].price, assets[3].currency);
    }
    
    // Show price movement indicator
    if (!assets[3].firstUpdate) {
      Serial.printf(" (%s)", assets[3].priceIncreased ? "UP" : "DOWN");
    }
    Serial.println();
    stockSuccess = true;
  } else {
    Serial.printf("Failed to fetch stock data: %s\n", apiClient.getLastError());
    // If we have existing price data, preserve it
    if (assets[3].price > 0.0) {
      assets[3].lastUpdated = "Update Failed";
      Serial.printf("Using cached stock price: %s: $%.2f %s\n", 
                    assets[3].symbol, assets[3].price, assets[3].currency);
      stockSuccess = true; // Don't treat this as a complete failure if we have cached data
    }
  }
  
  // Return true if at least one API call succeeded
  return cryptoSuccess || stockSuccess;
}

// Cycle through brightness levels when button A is pressed
void cycleBrightness() {
  // Move to next brightness level (cycle back to 0 if at end)
  currentBrightnessIndex = (currentBrightnessIndex + 1) % BRIGHTNESS_LEVEL_COUNT;
  
  // Apply new brightness using M5Unified API (works for all M5 devices)
  M5.Display.setBrightness(BRIGHTNESS_LEVELS[currentBrightnessIndex]);
  
  // Show brightness level briefly
  const uint8_t percentBrightness = (BRIGHTNESS_LEVELS[currentBrightnessIndex] * 100) / BRIGHTNESS_MAX;
  Serial.printf("Brightness changed to: %d/255 (%d%%, level %d)\n", 
                BRIGHTNESS_LEVELS[currentBrightnessIndex], percentBrightness, currentBrightnessIndex + 1);
  
  // Optional: Show brightness on screen briefly (uncomment if desired)
  /*
  M5.Lcd.fillRect(10, 10, 120, 30, TFT_BLACK);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.drawString("Brightness: " + String(brightnessPercent) + "%", 15, 20);
  delay(1000);
  */
}

// Setup NTP time synchronization for Eastern Time (EST/EDT auto-switching)
void setupTime() {
  Serial.println("Setting up time synchronization...");
  
  // Configure time for Eastern Time with automatic DST handling
  // EST: UTC-5, EDT: UTC-4 (automatically switches based on date)
  configTime(-5 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
  
  Serial.print("Waiting for NTP time sync");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 8 * 3600 * 2 && attempts < 20) { // Wait for valid time
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }
  Serial.println();
  
  if (now > 8 * 3600 * 2) {
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.printf("Time synchronized: %04d-%02d-%02d %02d:%02d:%02d ET\n",
                  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("Failed to synchronize time - market hours check may not work correctly");
  }
}

// Check if US stock market is open (9:05 AM - 4:05 PM ET, Monday-Friday)
bool isMarketOpen() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  
  // Check if it's a weekday (Monday=1, Friday=5)
  int dayOfWeek = timeinfo.tm_wday;
  if (dayOfWeek == 0 || dayOfWeek == 6) { // Sunday=0, Saturday=6
    Serial.println("Market closed: Weekend");
    return false;
  }
  
  // Check time range: 9:05 AM (09:05) to 4:05 PM (16:05) EDT
  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;
  int currentTimeInMinutes = currentHour * 60 + currentMinute;
  
  int marketOpenMinutes = 9 * 60 + 5;  // 9:05 AM = 545 minutes
  int marketCloseMinutes = 16 * 60 + 5; // 4:05 PM = 965 minutes
  
  bool isOpen = (currentTimeInMinutes >= marketOpenMinutes && currentTimeInMinutes <= marketCloseMinutes);
  
  Serial.printf("Current time: %02d:%02d ET, Market %s\n", 
                currentHour, currentMinute, isOpen ? "OPEN" : "CLOSED");
  
  return isOpen;
}
