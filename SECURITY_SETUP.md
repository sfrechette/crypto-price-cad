# Security Setup Guide

## Protecting Your Credentials

This guide helps you securely configure the Crypto & Stock Price Display project with your
API keys, WiFi credentials, and MQTT broker settings.

## Quick Setup Steps

### Step 1: Create secrets.h

```bash
# Copy the template to create your secrets file
cp secrets_template.h src/secrets.h
```

### Step 2: Configure Your Credentials

Edit `src/secrets.h` with your actual values:

```cpp
// WiFi Configuration
#define WIFI_SSID "Your_WiFi_Network_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// CoinMarketCap API Key (for BTC, ETH, XRP prices in CAD)
#define CMC_API_KEY "your-coinmarketcap-api-key-here"

// Financial Modeling Prep API Key (for MSFT stock price in USD)
#define FMP_API_KEY "your-financial-modeling-prep-api-key-here"

// MQTT Configuration (Home Assistant)
#define MQTT_BROKER "192.168.1.100"    // Your Home Assistant IP
#define MQTT_PORT 1883                  // Default Mosquitto port
#define MQTT_USER "mqtt_user"           // Your MQTT username
#define MQTT_PASSWORD "mqtt_password"   // Your MQTT password
```

### Step 3: Verify .gitignore Protection

Ensure your `.gitignore` includes:

```
src/secrets.h
*.log
.pio/
.vscode/
*.tmp
main_original_backup.cpp
```

## API Key Setup

### CoinMarketCap API (Cryptocurrency Data)

1. **Sign Up:** Visit [CoinMarketCap API](https://coinmarketcap.com/api/)
2. **Free Plan:** Get 10,000 calls/month (sufficient for this project)
3. **Get API Key:** From your dashboard after registration
4. **Add to secrets.h:** Replace `YOUR_COINMARKETCAP_API_KEY_HERE`

**API Usage:** Fetches BTC, ETH, XRP prices in CAD every 5 minutes

### Financial Modeling Prep API (Stock Data)

1. **Sign Up:** Visit [Financial Modeling Prep](https://financialmodelingprep.com/)
2. **Free Plan:** Available with limited calls
3. **Get API Key:** From your account dashboard
4. **Add to secrets.h:** Replace `YOUR_FINANCIAL_MODELING_PREP_API_KEY_HERE`

**API Usage:** Fetches MSFT stock price in USD during market hours only
(9:05 AM - 4:05 PM ET)

## MQTT Broker Setup (Home Assistant)

### Option 1: Home Assistant Mosquitto Add-on (Recommended)

1. **Install Add-on:**
   - Go to **Settings → Add-ons → Add-on Store**
   - Search for "Mosquitto broker"
   - Click **Install**

2. **Configure Add-on:**
   - Go to the Mosquitto add-on **Configuration** tab
   - Default config works for most setups

3. **Create MQTT User:**
   - Go to **Settings → People → Users**
   - Create a new user (e.g., `mqtt_user`)
   - This user will authenticate MQTT connections

4. **Start the Add-on:**
   - Go to **Info** tab and click **Start**
   - Enable **Start on boot**

5. **Configure secrets.h:**
   ```cpp
   #define MQTT_BROKER "192.168.1.100"  // Your HA IP address
   #define MQTT_PORT 1883
   #define MQTT_USER "mqtt_user"         // User created above
   #define MQTT_PASSWORD "user_password" // User's password
   ```

### Option 2: Standalone Mosquitto

1. **Install Mosquitto:**
   ```bash
   # Ubuntu/Debian
   sudo apt install mosquitto mosquitto-clients
   
   # macOS
   brew install mosquitto
   ```

2. **Configure Authentication:**
   ```bash
   # Create password file
   sudo mosquitto_passwd -c /etc/mosquitto/passwd mqtt_user
   
   # Add to mosquitto.conf
   allow_anonymous false
   password_file /etc/mosquitto/passwd
   ```

3. **Restart Mosquitto:**
   ```bash
   sudo systemctl restart mosquitto
   ```

### MQTT Configuration in secrets.h

```cpp
// MQTT Configuration (Home Assistant / Mosquitto)
#define MQTT_BROKER "192.168.1.100"    // Home Assistant or broker IP
#define MQTT_PORT 1883                  // Default: 1883, TLS: 8883
#define MQTT_USER "mqtt_user"           // Leave "" if no auth
#define MQTT_PASSWORD "mqtt_password"   // Leave "" if no auth
#define MQTT_CLIENT_ID "m5crypto"       // Unique client identifier
#define MQTT_TOPIC_PREFIX "m5crypto"    // Base topic for all messages
```

### Verifying MQTT Connection

After uploading firmware, check serial output:

```text
MQTT: Connecting to broker 192.168.1.100:1883
MQTT: User='mqtt_user', Pass length=12
MQTT: Connecting with authentication...
MQTT: Connected successfully!
MQTT: Published availability: online
```

**Common MQTT Errors:**

| Error Code | Meaning | Solution |
|------------|---------|----------|
| `MQTT_CONNECT_BAD_CREDENTIALS` | Wrong user/password | Check MQTT_USER and MQTT_PASSWORD |
| `MQTT_CONNECTION_TIMEOUT` | Can't reach broker | Check IP address and firewall |
| `MQTT_CONNECT_UNAUTHORIZED` | User not authorized | Check broker ACL settings |
| `MQTT_CONNECT_FAILED` | General failure | Check broker is running |

### Testing MQTT Connection

```bash
# Test publish (from another machine)
mosquitto_pub -h YOUR_HA_IP -u mqtt_user -P mqtt_pass -t "test" -m "hello"

# Monitor device topics
mosquitto_sub -h YOUR_HA_IP -u mqtt_user -P mqtt_pass -t "m5crypto/#" -v

# Expected output when device publishes:
# m5crypto/status online
# m5crypto/btc/state {"price":63245.67,"trend":"up","updated":"14:30:45"}
```

## Market Hours Intelligence

The system automatically:

- **Detects Market Hours:** Uses Eastern Time (EST/EDT) with automatic
  daylight saving
- **Smart API Calls:** Only fetches stock data during trading hours
- **Battery Optimization:** No stock API calls nights, weekends, or holidays
- **Price Persistence:** Last known stock price preserved until next
  trading day

## Security Best Practices

### ✅ DO

- **Keep secrets.h private** - Never commit to version control
- **Use strong WiFi passwords** - WPA3 recommended
- **Use MQTT authentication** - Never run broker with anonymous access
- **Monitor API usage** - Check your quotas regularly
- **Rotate keys periodically** - Update API keys every few months
- **Use different keys** - Don't reuse API keys across projects
- **Use dedicated MQTT user** - Don't use your main HA admin account

### ❌ DON'T

- **Share API keys publicly** - In forums, Discord, etc.
- **Commit secrets.h** - Always excluded by .gitignore
- **Use production keys in development** - Use separate keys if possible
- **Ignore rate limits** - Respect API quotas to avoid blocking
- **Expose MQTT to internet** - Keep broker on local network only
- **Use simple MQTT passwords** - Use strong, unique passwords

## If You Accidentally Commit Secrets

### Immediate Actions

1. **Revoke API keys** immediately in your dashboards
2. **Change MQTT password** in Home Assistant
3. **Generate new keys** with different values
4. **Remove from Git history:**

   ```bash
   git filter-branch --force --index-filter \
     'git rm --cached --ignore-unmatch src/secrets.h' \
     --prune-empty --tag-name-filter cat -- --all
   ```

5. **Force push** to remote (if applicable)
6. **Update secrets.h** with new keys

## API & MQTT Usage Monitoring

### Expected Usage (Per Day)

- **CoinMarketCap:** ~288 calls (every 5 minutes, 24/7)
- **Financial Modeling Prep:** ~60-72 calls (only during market hours)
- **MQTT Messages:** ~300-400 messages/day (prices + keepalives)
- **Total API:** ~350-360 calls/day (well within free tiers)

### Usage Optimization Features

- **Market Hours Detection:** No stock calls after 4:05 PM ET
- **Weekend Skipping:** No stock calls Saturday/Sunday
- **Error Handling:** Graceful degradation on API failures
- **Caching:** Displays last known prices during outages
- **MQTT Reconnection:** Automatic reconnection with 5-second intervals

## Network Security

### Recommended Network Setup

```text
┌─────────────────────────────────────────────────────────┐
│                    Home Network                         │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐ │
│  │   Router    │────│ Home        │────│  M5StickC   │ │
│  │  (Firewall) │    │ Assistant   │    │  Plus2      │ │
│  └─────────────┘    │ + Mosquitto │    └─────────────┘ │
│         │           └─────────────┘                     │
│         │                                               │
│    Internet                                             │
│    (APIs only)                                          │
└─────────────────────────────────────────────────────────┘
```

### Firewall Recommendations

- **Allow outbound:** HTTPS (443) for API calls
- **Allow local:** MQTT (1883) between device and broker
- **Block inbound:** No ports need to be exposed to internet
- **MQTT broker:** Only accessible from local network

### Physical Security

- **Secure mounting** - Prevent device theft
- **Power protection** - Use surge protectors
- **Environment** - Avoid extreme temperatures/humidity

## Troubleshooting

### Common Issues

#### "WiFi Connection Failed"

- Check SSID and password spelling
- Ensure 2.4GHz network (M5StickC Plus2 limitation)
- Verify network allows IoT devices

#### "API Errors"

- Verify API keys are correct
- Check API quotas haven't been exceeded
- Ensure internet connectivity

#### "MQTT Connection Failed"

- Verify broker IP address is correct
- Check MQTT username and password
- Ensure Mosquitto is running
- Check firewall isn't blocking port 1883

#### "Entities Not Appearing in Home Assistant"

- Verify MQTT integration is configured in HA
- Check Settings → Devices & Services → MQTT
- Restart Home Assistant after first device connection
- Check HA logs for MQTT discovery errors

#### "Time Sync Issues"

- Check NTP server accessibility
- Verify timezone shows "ET" not "EDT" in logs

#### "Stock Price Shows 0 or Market Closed"

- Normal behavior outside market hours
- Check if current time is 9:05 AM - 4:05 PM ET, Monday-Friday

## Maintenance

### Monthly Tasks

- **Check API usage** - Monitor quotas
- **Verify MQTT connectivity** - Ensure stable operation
- **Review Home Assistant logs** - Check for errors
- **Verify connectivity** - Ensure stable operation

### Quarterly Tasks

- **Update dependencies** - PlatformIO libraries
- **Rotate API keys** - Generate new keys
- **Change MQTT password** - Update credentials
- **Backup configuration** - Save working secrets template

## Support

### API Issues

- **CoinMarketCap:** [Documentation](https://coinmarketcap.com/api/documentation/v1/#section/Introduction)
- **Financial Modeling Prep:** [Documentation](https://financialmodelingprep.com/developer/docs)

### MQTT Issues

- **Mosquitto:** [Documentation](https://mosquitto.org/documentation/)
- **Home Assistant MQTT:** [Integration Docs](https://www.home-assistant.io/integrations/mqtt/)

### Hardware Issues

- **M5Stack Community:** [Forum](https://community.m5stack.com/)
- **GitHub Issues:** Create issue in this repository

---
