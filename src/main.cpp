/* Author: Stéphane Fréchette
 * Version: 1.0
 *
 * Description: This code is for the M5StickC Plus to display the current rates of Bitcoin (BTC), 
 * Ethereum (ETH), and Ripple (XRP) in Canadian dollars (CAD) using the CoinMarketCap API.
 * The rates are updated every 5 minutes. Each rate is displayed for 10 seconds before switching to the next rate.
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <M5StickCPlus.h>
#include <HTTPClient.h> 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <secrets.h>

#define CENTER 120

// Access the values defined in secrets.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* apiEndpoint = API_ENDPOINT; // CoinMarketCap API endpoint

unsigned long startTime = 0; // Variable to store the start time
const unsigned long interval = 300000; // Interval set for 5 minutes (5 * 60 * 1000 milliseconds)

void formatAndDisplay(float rate, const char* label, int yPos) {
  String num = String(rate, 2);  // Convert float to string with 2 decimal places
  int decimalPos = num.indexOf('.');  // Find the position of the decimal point
  
  String integerPart = num.substring(0, decimalPos);  // Extract integer part
  String decimalPart = num.substring(decimalPos);  // Extract decimal part including the dot
  
  // Format the integer part with commas
  String formattedNumber = "";
  int length = integerPart.length();
  int digits = 0;

  // Insert commas into the integer part
  for (int i = length - 1; i >= 0; i--) {
    formattedNumber = integerPart[i] + formattedNumber;
    digits++;
    if (digits % 3 == 0 && i != 0) {
      formattedNumber = ',' + formattedNumber;
    }
  }
  formattedNumber += decimalPart;  // Append the decimal part back to the formatted number

  M5.Lcd.drawString(formattedNumber, CENTER, yPos);  // Display the formatted rate
  Serial.println(label + String(" ") + formattedNumber);  // Output to serial
}

void setup() {
  Serial.begin(115200);

  startTime = millis(); // Initialize the start time

  WiFi.begin(ssid, password);

  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(32);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(1);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;  // Create a secure client
    client.setInsecure();     // Use this line if you do not want to manage SSL certificates

    HTTPClient http;
    http.begin(client, apiEndpoint);  // Start connection using secure client
    int httpCode = http.GET();        // Send the request

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();  // Get the response payload

      DynamicJsonDocument doc(16384);  // Allocate the JSON document
      deserializeJson(doc, payload);  // Parse JSON data

      const float rateBTC = doc["data"]["BTC"][0]["quote"]["CAD"]["price"];  // Extract BTC rate
      const char* updatedBTC = doc["data"]["BTC"][0]["quote"]["CAD"]["last_updated"];  // Extract BTC last updated

      const float rateETH = doc["data"]["ETH"][0]["quote"]["CAD"]["price"];  // Extract ETH rate
      const char* updatedETH = doc["data"]["ETH"][0]["quote"]["CAD"]["last_updated"];  // Extract ETH last updated
      
      const float rateXRP = doc["data"]["XRP"][0]["quote"]["CAD"]["price"];  // Extract XRP rate
      const char* updatedXRP = doc["data"]["XRP"][0]["quote"]["CAD"]["last_updated"];  // Extract XRP last updated

      while (millis() - startTime < interval) {
        // Display BTC
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setTextFont(2);
        M5.Lcd.setTextDatum(TC_DATUM);

        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("BTC CAD", CENTER, 8);
 
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(TFT_GREEN);
        formatAndDisplay(rateBTC, "BTC CAD:", 43);

        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString("Last updated:", CENTER, 83);
        Serial.print("Last updated: ");

        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString(updatedBTC, CENTER, 103);
        Serial.println(updatedBTC);

        //draw a frame around the text
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.drawRoundRect(4,4,236,130,6,TFT_DARKGREY);

        M5.Lcd.drawRoundRect(4,4,236,130,6,TFT_DARKGREY);

        delay(10000); 

        // Display ETH
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setTextFont(2);
        M5.Lcd.setTextDatum(TC_DATUM);

        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("ETH CAD", CENTER, 8);
   
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(TFT_GREEN);
        formatAndDisplay(rateETH, "ETH CAD:", 43);
 
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString("Last updated:", CENTER, 83);
        Serial.print("Last updated: ");

        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString(updatedETH, CENTER, 103);
        Serial.println(updatedETH);

        //draw a frame around the text
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.drawRoundRect(4,4,236,130,6,TFT_DARKGREY);

        delay(10000); 

        // Display XRP
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Lcd.setTextFont(2);
        M5.Lcd.setTextDatum(TC_DATUM);

        M5.Lcd.setTextSize(2);
        M5.Lcd.drawString("XRP CAD", CENTER, 8);

        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(TFT_GREEN);
        formatAndDisplay(rateXRP, "XRP CAD:", 43);
  
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString("Last updated:", CENTER, 83);
        Serial.print("Last updated: ");

        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.drawString(updatedXRP, CENTER, 103);
        Serial.println(updatedXRP);

        //draw a frame around the text
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.drawRoundRect(4,4,236,130,6,TFT_DARKGREY);
        delay(10000);
      }  
    } else {
      Serial.print("Error on HTTP request:");
      Serial.println(httpCode);
    } 
    http.end(); // Close connection
  }
  startTime = millis(); // Reset the start time to now
}