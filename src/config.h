#ifndef CONFIG_H
#define CONFIG_H

// Display configuration
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define CENTER_X (SCREEN_WIDTH / 2)

// Timing configuration
#define API_UPDATE_INTERVAL 300000  // 5 minutes in milliseconds
#define DISPLAY_DURATION 10000      // 10 seconds per crypto display
#define WIFI_CONNECT_TIMEOUT 20000  // 20 seconds WiFi timeout

// Display layout
#define ICON_SIZE 24
#define ICON_TEXT_GAP 8
#define ICON_Y_POS 12
#define TEXT_Y_POS 8
#define PRICE_Y_POS 43
#define UPDATE_LABEL_Y_POS 83
#define UPDATE_TIME_Y_POS 103

// Frame styling
#define FRAME_MARGIN 4
#define FRAME_CORNER_RADIUS 6

// Colors
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_TEXT TFT_WHITE
#define COLOR_PRICE TFT_YELLOW
#define COLOR_FRAME TFT_DARKGREY

#endif // CONFIG_H
