# Security Setup Guide

## 🛡️ Protecting Your Credentials

### Step 1: Create secrets.h
1. Copy `secrets_template.h` to `secrets.h`
2. Fill in your actual credentials:
   - WiFi SSID and password
   - CoinMarketCap API key

### Step 2: Add to .gitignore
Create or update `.gitignore` file:
```
secrets.h
*.log
.pio/
```

### Step 3: Get CoinMarketCap API Key
1. Visit [CoinMarketCap API](https://coinmarketcap.com/api/)
2. Sign up for free account
3. Get your API key from dashboard
4. Add to `secrets.h`

## 🚨 Important Security Notes

- **Never commit `secrets.h` to version control**
- **Never share your API keys publicly**
- **Use environment variables in production**
- **Rotate API keys regularly**

## 🔄 Using the Improved Code

To use the improved modular version:
1. Rename `main.cpp` to `main_original.cpp` (backup)
2. Rename `main_improved.cpp` to `main.cpp`
3. Update your `secrets.h` with the new template format
