// Template for secrets.h - Copy this to secrets.h and fill in your actual values
// DO NOT commit secrets.h to version control!

#ifndef SECRETS_TEMPLATE_H
#define SECRETS_TEMPLATE_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"
#define WIFI_CONNECT_TIMEOUT 30000  // 30 seconds timeout

// CoinMarketCap API Configuration (Cryptocurrency Data)
#define CMC_API_KEY "YOUR_COINMARKETCAP_API_KEY_HERE"
#define API_BASE_URL "https://pro-api.coinmarketcap.com/v2/cryptocurrency/quotes/latest"
#define API_SYMBOLS "BTC,ETH,XRP"
#define API_CONVERT "CAD"

// Financial Modeling Prep API Configuration (Stock Data)
#define FMP_API_KEY "YOUR_FINANCIAL_MODELING_PREP_API_KEY_HERE"
#define FMP_BASE_URL "https://financialmodelingprep.com/stable/quote"
#define STOCK_SYMBOL "MSFT"

// Construct the full API endpoints
#define API_ENDPOINT API_BASE_URL "?CMC_PRO_API_KEY=" CMC_API_KEY "&symbol=" API_SYMBOLS "&convert=" API_CONVERT
#define STOCK_ENDPOINT FMP_BASE_URL "?symbol=" STOCK_SYMBOL "&apikey=" FMP_API_KEY

// MQTT Configuration (Home Assistant / Mosquitto)
#define MQTT_BROKER "YOUR_HOME_ASSISTANT_IP"  // e.g., "192.168.1.100"
#define MQTT_PORT 1883                         // Default Mosquitto port
#define MQTT_USER ""                           // Leave empty if no auth
#define MQTT_PASSWORD ""                       // Leave empty if no auth
#define MQTT_CLIENT_ID "m5crypto"              // Unique client identifier
#define MQTT_TOPIC_PREFIX "m5crypto"           // Base topic for all messages

#endif // SECRETS_TEMPLATE_H
