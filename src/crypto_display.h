#ifndef CRYPTO_DISPLAY_H
#define CRYPTO_DISPLAY_H

#include <M5Unified.h>
#include "config.h"

// Structure to hold cryptocurrency and stock data
struct AssetData {
  const char* symbol;
  const char* name;
  float price;
  const char* lastUpdated;
  int iconX;
  int textX;
  int nameWidth;  // Approximate width in pixels for centering
  bool isStock;   // true for stocks, false for crypto
  const char* currency; // "CAD" for crypto, "USD" for stocks
  
  // Price movement tracking
  float previousPrice; // Track previous price for comparison
  bool priceIncreased; // true if price went up, false if down
  bool firstUpdate;    // true on first load (no arrow shown)
};

// Keep backward compatibility
typedef AssetData CryptoData;

// Cryptocurrency display class
class CryptoDisplay {
public:
  CryptoDisplay();
  
  // Initialize display settings
  void begin();
  
  // Display a single cryptocurrency or stock
  void displayAsset(const AssetData& asset);
  void displayCrypto(const CryptoData& crypto); // Backward compatibility
  
  // Display price movement arrow
  void displayPriceArrow(const AssetData& asset, int x, int y);
  
  // Display error message
  void displayError(const char* message);
  
  // Display WiFi connection status
  void displayWiFiStatus(const char* status);
  
private:
  // Helper functions
  void setupDisplaySettings();
  void drawFrame();
  void displayIcon(const char* symbol, int x, int y);
  void displayCenteredText(const char* text, int x, int y, int textSize, uint16_t color);
  void clearDisplayArea(int x, int y, int width, int height);
  String formatPrice(float price);
  void calculateCenterPosition(AssetData& asset);
};

#endif // CRYPTO_DISPLAY_H
