#include "display_manager.h"
#include "duco_logo_color.h"
#include <Adafruit_BME280.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFi.h>

Adafruit_BME280 bme;
float bme_temperature = NAN;
bool bme_ok = false;

String version = GUI_VERSION;

DisplayManager::DisplayManager() : tft(TFT_eSPI()) {
    currentScreen = 0;
    displayReady = false;
    lastButtonPress = 0;
    lastButtonState = HIGH;
    buttonPressed = false;
    lastDataUpdate = 0;
    lastScreenRefresh = 0;
    totalHashrate = 0.0;
    activeMiners = 0;
    totalMiners = 0;
    acceptanceRate = 0.0;
    balance = 0.0;
    balanceUSD = 0.0;
    ducoPrice = 0.0;
    mined24h = 0.0;
    apiDataValid = false;
    balance24hStart = -1.0f;
    balance24hStartTs = 0;
    minerVersion = "0.0";
    chartIndex = 0;
    lastBalance = -1.0;
    chartInitialized = false;
    for(int i = 0; i < CHART_POINTS; i++) chartData[i] = 0.0;
}


void DisplayManager::init(const char* mVersion) {
    // version
    this->minerVersion = String(mVersion);

    //  TFT
    tft.init();
    tft.setRotation(4);
    tft.setSwapBytes(true);

    // GPIO button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Wire.begin(DISPLAY_BME_SDA_PIN, DISPLAY_BME_SCL_PIN);

    //bme280
    if (bme.begin(DISPLAY_BME_ADDRESS)) {
        bme_ok = true;
        Serial.println("[DISPLAY] BME280 sensor detected");
    } else {
        bme_ok = false;
        Serial.println("[DISPLAY] BME280 sensor NOT detected");
    }

    //tft startup
    tft.fillRectVGradient(1, 1, 240, 240, COLOR_BG_1_LINEAR, COLOR_BG_2_LINEAR);
    drawLogo(102, 80);
    drawCenteredText("Loading...", 160, COLOR_WHITE, 1);
    delay(3000);
    displayReady = true;
    switchToScreen(0);
}


void DisplayManager::update() {
    if (!displayReady) return;
    if (millis() - lastDataUpdate > UPDATE_INTERVAL) {
        updateAPIData();
        if (bme_ok) bme_temperature = bme.readTemperature();
        lastDataUpdate = millis();
    }
}


void DisplayManager::handleButton() {
    bool state = !digitalRead(BUTTON_PIN); // INPUT_PULLUP
    static uint32_t debounceTs = 0;
    if (state && !lastButtonState && millis() - debounceTs > 40) {
        debounceTs = millis();
        currentScreen = (currentScreen + 1) % 4;
        switchToScreen(currentScreen);
    }
    lastButtonState = state;
}


void DisplayManager::switchToScreen(int screen) {
    if (screen < 0 || screen > 3) return;
    currentScreen = screen;
    refreshCurrentScreen();
}


void DisplayManager::refreshCurrentScreen() {
    tft.fillRectVGradient(1, 1, 240, 240, COLOR_BG_1_LINEAR, COLOR_BG_2_LINEAR);
    switch(currentScreen) {
        case 0: drawScreen1(); break;
        case 1: drawScreen2(); break;
        case 2: drawScreen3(); break;
        case 3: drawScreen4(); break;
    }
}



void DisplayManager::drawScreen1() {
    drawLogo(102, 13);
    drawCenteredText("Mined last 24h:", 59, COLOR_WHITE, 1);
    String minedStr = String(mined24h, 2);
    drawCenteredText(minedStr.c_str(), 73, COLOR_WHITE, 4);
    drawLine(40, 107, 200, 107, COLOR_WHITE);
    drawCenteredText("Total hashrate:", 115, COLOR_WHITE, 1);
    String hashrateStr = formatHashrate(totalHashrate);
    drawCenteredText(hashrateStr.c_str(), 129, COLOR_WHITE, 3);
    uint16_t acceptColor = acceptanceRate > 95.0 ? COLOR_GREEN :
        (acceptanceRate > 80.0 ? COLOR_YELLOW : COLOR_RED);
    String acceptStr = "Accepted: " + String(acceptanceRate, 2) + "%";
    drawCenteredText(acceptStr.c_str(), 167, acceptColor, 1);
    int barWidth = 160, barHeight = 3, barX = 40, barY = 160;
    drawProgressBar(barX, barY, barWidth, barHeight, acceptanceRate, acceptColor);
    drawLine(60, 180, 180, 180, COLOR_WHITE);
    drawCenteredText("Active miners:", 188, COLOR_WHITE, 1);
    String minersStr = String(activeMiners) + "/" + String(totalMiners);
    drawCenteredText(minersStr.c_str(), 204, COLOR_WHITE, 2);
}

void DisplayManager::drawScreen2() {
    drawLogo(102, 13);
    drawCenteredText("Balance:", 70, COLOR_WHITE, 1);
    String balanceStr = String(balance, 2);
    drawCenteredText(balanceStr.c_str(), 87, COLOR_WHITE, 4);
    String balanceUSDStr = "$" + String(balanceUSD, 6);
    drawCenteredText(balanceUSDStr.c_str(), 123, COLOR_GRAY, 2);
    drawLine(40, 145, 200, 145, COLOR_WHITE);
    drawCenteredText("DUCO Price:", 155, COLOR_WHITE, 1);
    String priceStr = "$" + String(ducoPrice, 8);
    drawCenteredText(priceStr.c_str(), 170, COLOR_WHITE, 2);
}

void DisplayManager::drawScreen3() {
    drawLogo(102, 13);
    drawChart();
    drawLine(60, 193, 180, 193, COLOR_WHITE);
    float currentMiningRate = chartData[(chartIndex - 1 + CHART_POINTS) % CHART_POINTS];
    String rateStr = String(currentMiningRate, 6) + " DUCO/10s";
    drawCenteredText(rateStr.c_str(), 200, COLOR_WHITE, 1);
}

void DisplayManager::drawScreen4() {
    drawLogo(102, 13);
    drawCenteredText("Status:", 65, COLOR_WHITE, 2);
    drawLine(40, 90, 200, 90, COLOR_WHITE);
    // Temperature
    if (bme_ok) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Temp: %.2f C", bme_temperature);
        drawCenteredText(buf, 100, bme_temperature <= 55.0 ? COLOR_GREEN : COLOR_RED, 1);
    } else {
        drawCenteredText("Temp: --.- C", 100, COLOR_RED, 1);
    }
    // WiFi status
    bool wifiOK = isWiFiConnected();
    drawCenteredText(("WIFI: " + String(wifiOK ? "OK" : "ERROR")).c_str(), 130, wifiOK ? COLOR_GREEN : COLOR_RED, 1);
    // API status
    drawCenteredText(("API: " + String(apiDataValid ? "OK" : "ERROR")).c_str(), 115, apiDataValid ? COLOR_GREEN : COLOR_RED, 1);
    // IP
    #ifdef USE_LAN
        IPAddress ip = ETH.localIP();
    #else
        IPAddress ip = WiFi.localIP();
    #endif
    drawCenteredText(("IP: " + ip.toString()).c_str(), 145, COLOR_WHITE, 1);
    // MINER/GUIs
    drawCenteredText(("Miner ver: " + minerVersion).c_str(), 160, COLOR_WHITE, 1);
    drawCenteredText(("GUI ver: " + version).c_str(), 175, COLOR_WHITE, 1);
    drawLine(60, 193, 180, 193, COLOR_WHITE);
    drawCenteredText("@spaceLAB", 200, COLOR_GRAY, 1);
    drawCenteredText("adamko.space", 210, COLOR_GRAY, 1);
}



void DisplayManager::drawChart() {
    int chartX = 60, chartY = 70, chartW = 120, chartH = 105;
    const int window = CHART_POINTS;
    float maxVal = 0.001f;
    for(int i = 0; i < window; i++) {
        int idx = (chartIndex - i - 1 + CHART_POINTS) % CHART_POINTS;
        if(chartData[idx] > maxVal) maxVal = chartData[idx];
    }
    // Grid
    for(int i = 1; i < 4; i++) {
        int y = chartY + (chartH * i / 4);
        tft.drawLine(chartX, y, chartX + chartW, y, COLOR_GRAY);
    }
    // Points/lines
    for(int i = 1; i < CHART_POINTS; i++) {
        if(!chartInitialized && i >= chartIndex) break;
        int x1 = chartX + ((i-1) * chartW / (CHART_POINTS-1));
        int y1 = chartY + chartH - (int)(chartData[i-1] / maxVal * chartH);
        int x2 = chartX + (i * chartW / (CHART_POINTS-1));
        int y2 = chartY + chartH - (int)(chartData[i] / maxVal * chartH);
        y1 = constrain(y1, chartY, chartY + chartH);
        y2 = constrain(y2, chartY, chartY + chartH);
        tft.drawLine(x1, y1, x2, y2, COLOR_WHITE);
        tft.fillCircle(x2, y2, 4, COLOR_WHITE);
    }
    // Scale:
    tft.setTextSize(1); tft.setTextColor(COLOR_WHITE);
    tft.setCursor(chartX + chartW + 10, chartY); tft.printf("%.2f", maxVal);
    tft.setCursor(chartX + chartW + 10, chartY + chartH - 8); tft.print("0.00");
}


void DisplayManager::drawLogo(int x, int y) {
    tft.pushImage(x, y,
        DUCO_LOGO_WIDTH, DUCO_LOGO_HEIGHT,
        DUCO_LOGO_37x37,
        DUCO_LOGO_MASK);
}

void DisplayManager::drawCenteredText(const char* text, int y, uint16_t color, uint8_t textSize) {
    tft.setTextSize(textSize);
    tft.setTextColor(color);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(text, SCREEN_WIDTH / 2, y);
}

void DisplayManager::drawRightAlignedText(const char* text, int x, int y, uint16_t color, uint8_t textSize) {
    tft.setTextSize(textSize);
    tft.setTextColor(color);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(text, x, y);
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, float percentage, uint16_t color) {
    tft.drawRect(x, y, width, height, COLOR_WHITE);
    tft.fillRect(x + 1, y + 1, width - 2, height - 2, COLOR_BLACK);
    int fillWidth = (int)((width - 2) * percentage / 100.0f);
    if (fillWidth > 0) tft.fillRect(x + 1, y + 1, fillWidth, height - 2, color);
}

void DisplayManager::drawLine(int32_t xs, int32_t ys, int32_t xe, int32_t ye, uint32_t color) {
    tft.drawLine(xs, ys, xe, ye, COLOR_WHITE);
}

String DisplayManager::formatHashrate(float hashrate) {
    if (hashrate >= 1000000.0)   return String(hashrate / 1000000.0, 2) + " MH/s";
    else if (hashrate >= 1000.0) return String(hashrate / 1000.0, 2) + " kH/s";
    else                         return String((int)hashrate) + " H/s";
}

String DisplayManager::formatBalance(float balance, int decimals) {
    return String(balance, decimals);
}

//Update API

void DisplayManager::updateAPIData() {
    if (!isWiFiConnected()) {
        apiDataValid = false;
        Serial.println("[DISPLAY] WiFi not connected, skipping API update");
        return;
    }
    Serial.println("[DISPLAY] Starting updateAPIData()");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    http.setConnectTimeout(3000);
    http.setTimeout(5000);
    bool balanceSuccess = false, userDataSuccess = false, statisticsSuccess = false;

    // Balance
    String balanceUrl = "https://server.duinocoin.com/balances/" + String(DUCO_USER);
    http.begin(client, balanceUrl);
    http.addHeader("Accept", "application/json");
    Serial.println("[DISPLAY] Requesting balance from: " + balanceUrl);
    int httpCode = http.GET();
    Serial.println("[DISPLAY] Balance API response code: " + String(httpCode));
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            if (doc["success"].as<bool>()) {
                balance = doc["result"]["balance"].as<float>();
                balanceSuccess = true;
                Serial.printf("[DISPLAY] Balance updated: %.4f DUCO\n", balance);
            }
        }
    }
    http.end();

    // User data
    String userUrl = "https://server.duinocoin.com/users/" + String(DUCO_USER);
    http.begin(client, userUrl);
    http.addHeader("Accept", "application/json");
    Serial.println("[DISPLAY] Requesting user data from: " + userUrl);
    httpCode = http.GET();
    Serial.println("[DISPLAY] User API response code: " + String(httpCode));
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096);
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            if (doc["success"].as<bool>()) {
                JsonArray miners = doc["result"]["miners"];
                totalHashrate = 0.0;
                activeMiners = 0;
                totalMiners = miners.size();
                float totalAccepted = 0, totalSubmitted = 0;
                for (JsonObject miner : miners) {
                    float minerHashrate = miner["hashrate"].as<float>();
                    float accepted = miner["accepted"].as<float>();
                    float rejected = miner["rejected"].as<float>();
                    totalHashrate += minerHashrate;
                    if (minerHashrate > 0) activeMiners++;
                    totalAccepted += accepted;
                    totalSubmitted += (accepted + rejected);
                }
                acceptanceRate = (totalSubmitted > 0) ? (totalAccepted / totalSubmitted) * 100.0 : 0.0;
                userDataSuccess = true;
                Serial.printf("[DISPLAY] User data updated - Hashrate: %.1f kH/s, Active: %d/%d, Acceptance: %.1f%%\n", totalHashrate/1000.0, activeMiners, totalMiners, acceptanceRate);
            }
        }
    }
    http.end();

    // Price DUCO
    String statsUrl = "https://server.duinocoin.com/statistics";
    http.begin(client, statsUrl);
    http.addHeader("Accept", "application/json");
    Serial.println("[DISPLAY] Requesting statistics from: " + statsUrl);
    httpCode = http.GET();
    Serial.println("[DISPLAY] Statistics API response code: " + String(httpCode));
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        if (deserializeJson(doc, payload) == DeserializationError::Ok) {
            ducoPrice = doc["Duco price"].as<float>();
            balanceUSD = balance * ducoPrice;
            statisticsSuccess = true;
            Serial.printf("[DISPLAY] DUCO price updated: $%.8f\n", ducoPrice);
        }
    }
    http.end();

    // 24h BLOCK
    unsigned long now = millis();
    if (balance24hStart < 0 || now - balance24hStartTs >= 86400000UL) {
        balance24hStart = balance;
        balance24hStartTs = now;
    }
    mined24h = max(0.0f, balance - balance24hStart);

    // Update chart:
    updateChart();
    lastBalance = balance;
    if (!balanceSuccess) {
        chartData[chartIndex] = 0;
        chartIndex = (chartIndex + 1) % CHART_POINTS;
    }

    apiDataValid = balanceSuccess && userDataSuccess && statisticsSuccess;
    if (apiDataValid) {
        Serial.println("[DISPLAY] All API data updated successfully");
        forceScreenUpdate(); // natychmiast odśwież
    } else {
        Serial.println("[DISPLAY] Some API data failed to update");
        forceScreenUpdate();
    }
}

void DisplayManager::updateChart() {
    if (lastBalance < 0) return;
    float minedAmount = balance - lastBalance;
    if (minedAmount < 0) minedAmount = 0;
    chartData[chartIndex] = minedAmount;
    chartIndex = (chartIndex + 1) % CHART_POINTS;
    if (!chartInitialized && chartIndex == 0) chartInitialized = true;
    Serial.printf("[DISPLAY] Chart updated: %.6f DUCO mined\n", minedAmount);
}



bool DisplayManager::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void DisplayManager::forceScreenUpdate() {
    refreshCurrentScreen();
}
bool DisplayManager::isInitialized() {
    return displayReady;
}

// Metoda pomocnicza do aktualizacji danych kopiowanych z minera
//void DisplayManager::setMiningData(float hashrate, int active, int total, float acceptance) {
//    totalHashrate = hashrate;
 //   activeMiners = active;
 //   totalMiners = total;
 //   acceptanceRate = acceptance;
//}
