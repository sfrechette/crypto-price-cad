#include "crypto_display.h"
#include "icons.h"

CryptoDisplay::CryptoDisplay() {
}

void CryptoDisplay::begin() {
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(32);
  M5.Lcd.fillScreen(COLOR_BACKGROUND);
  setupDisplaySettings();
}

void CryptoDisplay::setupDisplaySettings() {
  M5.Lcd.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
}

void CryptoDisplay::displayAsset(const AssetData& asset) {
  // Only clear and redraw when switching to a different cryptocurrency
  static String lastSymbol = "";
  static String lastPrice = "";
  static String lastUpdated = "";
  
  bool assetChanged = (lastSymbol != String(asset.symbol));
  String currentPrice = formatPrice(asset.price);
  bool priceChanged = (lastPrice != currentPrice);
  bool timeChanged = (lastUpdated != String(asset.lastUpdated));
  
  if (assetChanged) {
    // Full screen refresh when switching assets
    M5.Lcd.fillScreen(COLOR_BACKGROUND);
    setupDisplaySettings();
    
    // Calculate centered positions
    AssetData centeredAsset = asset;
    calculateCenterPosition(centeredAsset);
    
    // Display icon centered with text vertically
    // Text is at Y=8 with height 16 (size 2), so text center is at Y=16
    // Icon is 24px tall, so to center icon with text center: iconY = 16 - 12 = 4
    // Adding a bit more for better visual balance
    int iconY = TEXT_Y_POS + 4; // Position icon to be centered with text middle
    displayIcon(centeredAsset.symbol, centeredAsset.iconX, iconY);
    
    // Display asset name
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    M5.Lcd.drawString(centeredAsset.name, centeredAsset.textX, TEXT_Y_POS);
    
    // Display static labels
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString("Last updated:", CENTER_X, UPDATE_LABEL_Y_POS);
    
    // Draw frame
    drawFrame();
    
    lastSymbol = String(asset.symbol);
  }
  
  // Update price if it changed (without clearing screen)
  if (priceChanged || assetChanged) {
    // Clear only price area - avoid frame edges
    M5.Lcd.fillRect(FRAME_MARGIN + 2, PRICE_Y_POS - 5, 
                   SCREEN_WIDTH - (FRAME_MARGIN * 2) - 4, 25, COLOR_BACKGROUND);
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(COLOR_PRICE, COLOR_BACKGROUND);
    
    // Calculate actual width of price text (size 2 font)
    int priceWidth = M5.Lcd.textWidth(currentPrice);
    int arrowSpacing = 8; // Space between price and arrow
    int totalWidth = priceWidth + arrowSpacing + ARROW_WIDTH;
    
    // Center the price+arrow combination
    int priceX = CENTER_X - (totalWidth / 2);
    int arrowX = priceX + priceWidth + arrowSpacing;
    
    // Draw price text (left-aligned from calculated position)
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.drawString(currentPrice, priceX, PRICE_Y_POS);
    
    // Display price movement arrow (vertically centered with price text)
    // Text size 2 is ~16px height, arrow is 12px height
    // Lower the arrow more to center with price value
    int arrowY = PRICE_Y_POS + 6;  // Lower positioning for better centering
    displayPriceArrow(asset, arrowX, arrowY);
    
    lastPrice = currentPrice;
  }
  
  // Update timestamp if it changed (without clearing screen)
  if (timeChanged || assetChanged) {
    // Clear only timestamp area - avoid frame edges
    M5.Lcd.fillRect(FRAME_MARGIN + 2, UPDATE_TIME_Y_POS - 5, 
                   SCREEN_WIDTH - (FRAME_MARGIN * 2) - 4, 20, COLOR_BACKGROUND);
    
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.drawString(asset.lastUpdated, CENTER_X, UPDATE_TIME_Y_POS);
    
    lastUpdated = String(asset.lastUpdated);
  }
  
  // Always redraw frame to ensure it's complete (lightweight operation)
  drawFrame();
  
  // Serial output only on changes
  if (assetChanged || priceChanged) {
    Serial.printf("%s %s: %s - Updated: %s\n", 
                  asset.symbol, 
                  asset.currency,
                  currentPrice.c_str(), 
                  asset.lastUpdated);
  }
}

// Backward compatibility wrapper
void CryptoDisplay::displayCrypto(const CryptoData& crypto) {
  displayAsset(crypto);
}

void CryptoDisplay::displayPriceArrow(const AssetData& asset, int x, int y) {
  // Only show arrow after first update (when we have previous price to compare)
  if (asset.firstUpdate) {
    return; // No arrow on first load
  }
  
  if (asset.priceIncreased) {
    // Green up arrow
    M5.Lcd.pushImage(x, y, ARROW_WIDTH, ARROW_HEIGHT, up_arrow);
  } else {
    // Red down arrow  
    M5.Lcd.pushImage(x, y, ARROW_WIDTH, ARROW_HEIGHT, down_arrow);
  }
}

void CryptoDisplay::displayError(const char* message) {
  M5.Lcd.fillScreen(COLOR_BACKGROUND);
  setupDisplaySettings();
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_RED);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("ERROR", CENTER_X, 40);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_TEXT);
  M5.Lcd.drawString(message, CENTER_X, 70);
  
  Serial.printf("ERROR: %s\n", message);
}

void CryptoDisplay::displayWiFiStatus(const char* status) {
  M5.Lcd.fillScreen(COLOR_BACKGROUND);
  setupDisplaySettings();
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(TFT_YELLOW);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("WiFi", CENTER_X, 40);
  
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(COLOR_TEXT);
  M5.Lcd.drawString(status, CENTER_X, 70);
  
  Serial.printf("WiFi: %s\n", status);
}

void CryptoDisplay::drawFrame() {
  // Draw a complete border frame
  M5.Lcd.drawRoundRect(
    FRAME_MARGIN, 
    FRAME_MARGIN, 
    SCREEN_WIDTH - (FRAME_MARGIN * 2), 
    SCREEN_HEIGHT - (FRAME_MARGIN * 2), 
    FRAME_CORNER_RADIUS, 
    COLOR_FRAME
  );
  
  // Draw a second frame line for better visibility (optional)
  M5.Lcd.drawRoundRect(
    FRAME_MARGIN + 1, 
    FRAME_MARGIN + 1, 
    SCREEN_WIDTH - (FRAME_MARGIN * 2) - 2, 
    SCREEN_HEIGHT - (FRAME_MARGIN * 2) - 2, 
    FRAME_CORNER_RADIUS - 1, 
    COLOR_FRAME
  );
}

void CryptoDisplay::displayIcon(const char* symbol, int x, int y) {
  if (strcmp(symbol, "BTC") == 0) {
    M5.Lcd.pushImage(x, y, BTC_ICON_WIDTH, BTC_ICON_HEIGHT, btc_icon);
  } 
  else if (strcmp(symbol, "ETH") == 0) {
    M5.Lcd.pushImage(x, y, ETH_ICON_WIDTH, ETH_ICON_HEIGHT, eth_icon);
  }
  else if (strcmp(symbol, "XRP") == 0) {
    M5.Lcd.pushImage(x, y, XRP_ICON_WIDTH, XRP_ICON_HEIGHT, xrp_icon);
  }
  else if (strcmp(symbol, "MSFT") == 0) {
    M5.Lcd.pushImage(x, y, MSFT_ICON_WIDTH, MSFT_ICON_HEIGHT, msft_icon);
  }
}

String CryptoDisplay::formatPrice(float price) {
  String num = String(price, 2);
  int decimalPos = num.indexOf('.');
  
  if (decimalPos == -1) return num; // No decimal point found
  
  String integerPart = num.substring(0, decimalPos);
  String decimalPart = num.substring(decimalPos);
  
  // Format integer part with commas
  String formatted = "";
  int length = integerPart.length();
  
  for (int i = 0; i < length; i++) {
    if (i > 0 && (length - i) % 3 == 0) {
      formatted += ",";
    }
    formatted += integerPart[i];
  }
  
  return formatted + decimalPart;
}

void CryptoDisplay::clearDisplayArea(int x, int y, int width, int height) {
  M5.Lcd.fillRect(x, y, width, height, COLOR_BACKGROUND);
}

void CryptoDisplay::calculateCenterPosition(AssetData& asset) {
  // Calculate total width: icon + gap + text
  int totalWidth = ICON_SIZE + ICON_TEXT_GAP + asset.nameWidth;
  
  // Center the combination
  asset.iconX = (SCREEN_WIDTH - totalWidth) / 2;
  asset.textX = asset.iconX + ICON_SIZE + ICON_TEXT_GAP;
}
