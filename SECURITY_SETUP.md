# Security Setup Guide

## 🛡️ Protecting Your Credentials

This guide helps you securely configure the Crypto & Stock Price Display project with your API keys and WiFi credentials.

## 📋 Quick Setup Steps

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

## 🔑 API Key Setup

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

**API Usage:** Fetches MSFT stock price in USD during market hours only (9:05 AM - 4:05 PM ET)

## 🕐 Market Hours Intelligence

The system automatically:
- **Detects Market Hours:** Uses Eastern Time (EST/EDT) with automatic daylight saving
- **Smart API Calls:** Only fetches stock data during trading hours
- **Battery Optimization:** No stock API calls nights, weekends, or holidays
- **Price Persistence:** Last known stock price preserved until next trading day

## 🔒 Security Best Practices

### ✅ DO:
- **Keep secrets.h private** - Never commit to version control
- **Use strong WiFi passwords** - WPA3 recommended
- **Monitor API usage** - Check your quotas regularly
- **Rotate keys periodically** - Update API keys every few months
- **Use different keys** - Don't reuse API keys across projects

### ❌ DON'T:
- **Share API keys publicly** - In forums, Discord, etc.
- **Commit secrets.h** - Always excluded by .gitignore
- **Use production keys in development** - Use separate keys if possible
- **Ignore rate limits** - Respect API quotas to avoid blocking

## 🚨 If You Accidentally Commit Secrets

### Immediate Actions:
1. **Revoke API keys** immediately in your dashboards
2. **Generate new keys** with different values
3. **Remove from Git history:**
   ```bash
   git filter-branch --force --index-filter \
   'git rm --cached --ignore-unmatch src/secrets.h' \
   --prune-empty --tag-name-filter cat -- --all
   ```
4. **Force push** to remote (if applicable)
5. **Update secrets.h** with new keys

## 📊 API Usage Monitoring

### Expected Usage (Per Day):
- **CoinMarketCap:** ~288 calls (every 5 minutes, 24/7)
- **Financial Modeling Prep:** ~60-72 calls (only during market hours)
- **Total:** ~350-360 calls/day
- **Monthly:** ~10,500-11,000 calls (well within free tiers)

### Usage Optimization Features:
- **Market Hours Detection:** No stock calls after 4:05 PM ET
- **Weekend Skipping:** No stock calls Saturday/Sunday
- **Error Handling:** Graceful degradation on API failures
- **Caching:** Displays last known prices during outages

## 🔧 Troubleshooting

### Common Issues:

**"WiFi Connection Failed"**
- Check SSID and password spelling
- Ensure 2.4GHz network (M5StickC Plus limitation)
- Verify network allows IoT devices

**"API Errors"**
- Verify API keys are correct
- Check API quotas haven't been exceeded
- Ensure internet connectivity

**"Time Sync Issues"**
- Check NTP server accessibility
- Verify timezone shows "ET" not "EDT" in logs

**"Stock Price Shows 0 or Market Closed"**
- Normal behavior outside market hours
- Check if current time is 9:05 AM - 4:05 PM ET, Monday-Friday

## 📱 Device Security

### Physical Security:
- **Secure mounting** - Prevent device theft
- **Power protection** - Use surge protectors
- **Environment** - Avoid extreme temperatures/humidity

### Network Security:
- **Dedicated IoT network** - Separate from main devices
- **Firewall rules** - Restrict unnecessary outbound connections
- **Regular updates** - Keep firmware current

## 🔄 Maintenance

### Monthly Tasks:
- **Check API usage** - Monitor quotas
- **Verify connectivity** - Ensure stable operation
- **Review logs** - Check for errors or issues

### Quarterly Tasks:
- **Update dependencies** - PlatformIO libraries
- **Rotate API keys** - Generate new keys
- **Backup configuration** - Save working secrets template

## 📞 Support

### API Issues:
- **CoinMarketCap:** [Support Center](https://coinmarketcap.com/api/support/)
- **Financial Modeling Prep:** [Documentation](https://financialmodelingprep.com/developer/docs)

### Hardware Issues:
- **M5Stack Community:** [Forum](https://community.m5stack.com/)
- **GitHub Issues:** Create issue in this repository

---

**⚠️ Remember: Security is your responsibility. Keep your credentials safe!**