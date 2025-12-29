#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#include "display_manager_settings.h"

#define BUTTON_PIN      5
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   240
#define CHART_POINTS    6
#define UPDATE_INTERVAL 10000 // ms

#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_ORANGE    0xfb00
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_RED       0xF800
#define COLOR_GRAY      0x4a49
#define COLOR_YELLOW    0xFFE0
#define COLOR_BG_1_LINEAR 0xfd81
#define COLOR_BG_2_LINEAR 0xfa48

class DisplayManager {
public:
    DisplayManager();
    void init(const char* minerVersion = GUI_VERSION);

    void update();
    void handleButton();
    bool isInitialized();
    void forceScreenUpdate();

    void setMiningData(float hashrate, int active, int total, float acceptance);

private:
    TFT_eSPI tft;
    int currentScreen;
    bool displayReady;
    unsigned long lastButtonPress;
    bool lastButtonState;
    bool buttonPressed;
    unsigned long lastDataUpdate;
    unsigned long lastScreenRefresh;

    // Mining/API data
    float totalHashrate, acceptanceRate, balance, balanceUSD, ducoPrice, mined24h;
    int activeMiners, totalMiners;
    bool apiDataValid;
    float balance24hStart;
    unsigned long balance24hStartTs;
    float chartData[CHART_POINTS];
    int chartIndex;
    float lastBalance;
    bool chartInitialized;
    String minerVersion;

    // GUI
    void switchToScreen(int screen);
    void refreshCurrentScreen();
    void drawScreen1();
    void drawScreen2();
    void drawScreen3();
    void drawScreen4();

    void drawLogo(int x, int y);
    void drawCenteredText(const char* text, int y, uint16_t color, uint8_t textSize = 1);
    void drawRightAlignedText(const char* text, int x, int y, uint16_t color, uint8_t textSize = 1);
    void drawProgressBar(int x, int y, int width, int height, float percentage, uint16_t color);
    void drawLine(int32_t xs, int32_t ys, int32_t xe, int32_t ye, uint32_t color);
    void drawChart();
    String formatHashrate(float hashrate);
    String formatBalance(float balance, int decimals = 2);

    // Data
    void updateAPIData();
    void updateChart();
    bool isWiFiConnected();
};

#endif // DISPLAY_MANAGER_H
