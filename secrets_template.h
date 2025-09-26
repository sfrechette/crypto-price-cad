// Template for secrets.h - Copy this to secrets.h and fill in your actual values
// DO NOT commit secrets.h to version control!

#ifndef SECRETS_H
#define SECRETS_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"

// CoinMarketCap API Configuration
#define CMC_API_KEY "YOUR_COINMARKETCAP_API_KEY_HERE"
#define API_BASE_URL "https://pro-api.coinmarketcap.com/v2/cryptocurrency/quotes/latest"
#define API_SYMBOLS "BTC,ETH,XRP"
#define API_CONVERT "CAD"

// Construct the full API endpoint
#define API_ENDPOINT API_BASE_URL "?CMC_PRO_API_KEY=" CMC_API_KEY "&symbol=" API_SYMBOLS "&convert=" API_CONVERT

#endif // SECRETS_H
