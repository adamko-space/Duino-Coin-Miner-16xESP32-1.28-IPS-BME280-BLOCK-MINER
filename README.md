
![App Screenshot](https://placehold.co/800x350)
# Duino-Coin Miner - 16xESP32 + 1,28 IPS + BME280 - BLOCK MINER
Hello. I would like to present the duino-coin miner I made. The name block miner comes from block design. You can duplicate the module and create smart farm of miners. The display shows us mining and user statistics. The BME280 sensor controls the ambient temperature between the modules card ESP32. API and sensor data are refreshed with a 10-second interval and drawn on the display after each update.

## 1. BLOCK Miner
16xESP32 DevKit - ~189 kH/s * 16 = **~3.02 MH/s** - Hashrate

Addition hardware:

- Rounded TFT IPS 1.28″ GC9A01 240x240px connected via **SPI** bus,
- BME280 sensor connected to the **I2C** bus.
## 2. GUI Features

![App Screenshot](https://placehold.co/800x250)

I designed a graphical interface consistent with the Duino Coin's colors. It has four data screens. The screens automatically change every minute – you can change the time in display_manager.h.

If you want to completely disable the not change auto screen, you can do it by setting 0 in:
```bash
#define SCREEN_AUTO_SWITCH_ENABLED 1 [change to 0 to disable]
```
 in **display_manager_setting.h** on line 30. 

 
 
 **In GUI**: If you want to disable auto change screen while the program is running, **press and hold the key for 0.5 seconds when switching to screen 4.** On screen 4 you will see the auto change status.
 


Screens:
- **Screen 1** - MINIG mined (*last24h*), total hashrate, acteptance rate,active/total miners
- **Screen 2** - USER - User balance DUCO & USD, DUCO Price
- **Screen 3** - CHART - 6 point chart - 60 sek - 10sek refresh
- **Screen 4** - STATUS - TEMPERATURE from BME280 sensor, PWM Cycle %, WIFI and API status, IP adres, miners version, GUI version, auto change screen status




## 3. Software
3.1 We install the program - the official miner + our extension on one of the ESP32s to which we will connect the rest - LCD, button, sensor and fan.

**display.update()** and **display.handleButton()** are executed in **void loop()** in the main miner file, which causes a conflict every time the button is called, because the **mine()** function fills 100% of the esp32 cores. I want both cores to mine. It's not a problem for me. The most important is the minig and the LCD is only for information and statistics. It would be simple and work :)

**Important:**
```bash
  If you install the program on one of your miners, its performance will drop by a maximum of 15 kHh/s
```

**Important:**
```bash
  When changing the version of the official miner,
  follow the steps from point 8. Installation. once again.
```


## 4. API

- Rest API - ducoprice -https://server.duinocoin.com/statistics

- Rest API for user statistic: https://server.duinocoin.com/users/YOUR_USER_NAME

## 5. FTF_eSPI libary
Remember to correctly edit the configuration file for your display model. To do this, please visit the library's official github page. The library supports multiple displays, e.g. ILI9341, GC9A01, ST7735, ST7789, etc.

Official link: [TFT_eSPI Libary Github](https://github.com/Bodmer/TFT_eSPI)
```bash
User_Setup.h -  in tft_espi libary folder in Arduino libary folder
```

Uncoment if you use **GC9A01** TFT:
```bash

#define GC9A01_DRIVER

```
```bash

#define TFT_WIDTH  240
#define TFT_HEIGHT  240

```
```bash

#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4 

```
```bash

#define LOAD_GLCD 
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7 
#define LOAD_FONT8
#define LOAD_GFXFF 
#define SMOOTH_FONT

```
```bash

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  2000000
#define SPI_TOUCH_FREQUENCY  2500000

```



## 6. BME280 Sensor

If you need to change the sensor address, go to read 10. Installation and point 5.

Official link: [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
```bash
REMEMBER: Some sensors do not tolerate 5V voltage unless they have a voltage regulator and must be powered by 3.3V.
```

## 7. Fan control

Use MOSFET TYP-N to control 2 x 4020 fan.

*Connection:*
![App Screenshot](/img/schematic-fun-control.jpg)

PWM and Temperature setitings change in **display_manager_setting.h**. Adjust to your requirements, the fans used, and the ambient conditions. 
```bash

//  PWM duty cycle [0–255]
#define PWM_MIN 70    
#define PWM_MAX 220

// Temperature range
#define TEMP_MIN 35.0
#define TEMP_MAX 55.0

```


in the function ***void DisplayManager::updateFanControl()**, the program creates a map from the min and max PWM values and min and max temperature.
```bash 
pwmValue = map(temp, TEMP_MIN, TEMP_MAX, PWM_MIN, PWM_MAX);
```
Example intial config:

```bash 

Range Temperature

TEMP_MIN=30.0         
TEMP_MAX=50.0      

Range PWM (initial setting) (max 255)

PWM_MIN=70
PWM_MAX=220            

Range PWM % (initial setting) (max 255-100%)

27.45% - 86.27%

```


In the GUI, the display on screen 4 shows the percentage of the PWM duty cycle. 0-100% in your range PWM_MIN and PWM_MAX. Both the variable and the duty cycle control are refreshed every 10 seconds in the **display.update()** function.




## 8. Need parts

| INPUT             | Link                                                               |
| ----------------- | ------------------------------------------------------------------ |
| ESP32 - DevKit - CH340 - 30PIN x16 | https://www.aliexpress.com/item/1005004476867346.html |
| 1,28-round LCD TFT 240x240 IPS C9A01 | https://www.aliexpress.com/item/1005009070161632.html |
|  BME280 Sesor +3.3V/+5V  | https://pl.aliexpress.com/item/1005008511564094.html |
| Male goldpin 15 PIN - angle | https://www.aliexpress.com/item/1005007569841697.html|
| Female headers - 15 PIN |https://www.aliexpress.com/item/4001198421663.html |
|  PCB 9x15cm  | https://www.aliexpress.com/item/1005007977006793.html |
|  2 PIN PCB Connector  | https://pl.aliexpress.com/item/1005007533752837.html |
| 2x 4020 5V fan    | https://pl.aliexpress.com/item/4001345654847.html |
| Transistor MOSFET IRLR3705ZTRBF (N-chanel low-level)    | near electronic store |
| Resistor 220 omhs   | near electronic store |
| Resistor 10k    | near electronic store |
| 2x Didode shotky IN5819   | near electronic store |
| Button (6x6)   | my collection |
| Wire | my collection |
| Alluminium pipe fi.5mm/ inner 3mm | my collection |
| 3mm steel threaded rod | my collection |
| 3mm hex nut | my collection |
| printed 3D case | friends printer |


## 9. Connection to ESP32
![App Screenshot](/img/esp32-connection.jpg)

| INPUT             | GPIO PIN (ESP32 devkit - 30pin)                                                               |
| ----------------- | ------------------------------------------------------------------ |
| LCD 1,28 - C9A01: **TFT_CS**  | 15 |
| LCD 1,28 - C9A01: **TFT_DC** | 2 |
| LCD 1,28 - C9A01: **TFT_RST** | 4 |
| LCD 1,28 - C9A01: **TFT_MOSI** | 23 |
| LCD 1,28 - C9A01: **TFT_SLCK**  | 18 |
| LCD 1,28 - C9A01: **VCC**  | +3.3V |
| LCD 1,28 - C9A01: **GND**  | GND |
| Button: **INTERNAL_PULLUP** - short to GND | 5 |
| BME280 - **SCL**  | 19 |
| BME280 - **SDA** | 21 | 
| BME280 - **VCC** | +3.3V / +5V* | 
| BME280 - **GND** | GND | 
| FUN - PWM | 22 | 


*Only the BME280 version which has a built-in voltage regulator and can be powered by +5V.


## 10. Required library

```bash

  <Arduino.h>
  <HTTPClient.h>
  <WiFiClientSecure.h>
  <ArduinoJson.h>
  <WiFi.h>
  <Wire.h>
  <Adafruit_BME280.h> and libraries required from Adafruit
  <TFT_eSPI.h>
  <time.h>
  <driver/ledc.h>

```


## 11. Installation

1. *File:* **Copy 3 files from repositores to ESP minier code folder**

```bash

  display_manager.h,
  display_manager.cpp,
  display_manager_setting.h,
  duco_logo_color.h

```


2. *Edit:* **Add line:** after **#include "Settings.h"** in ESP_code.ino - miner files

```bash
#include "display_manager.h"
DisplayManager display;
```

3. *Edit:* **Add line:** in **void loop()** in ESP_code.ino - miner files

```bash
  display.init(SOFTWARE_VERSION);
```



4. *Edit:* **Add line:** in **void loop()** in ESP_code.ino - miner files

```bash

  display.updateAutoScreen();
  display.handleButton();
  display.update()

```


5. *Edit:* **change “YOUR_USER” to your username** in **display_manager_setting.h** file on line 8.

```bash
 #define DUCO_USER "YOUR_USER"
```


6. *Edit:*change I2C **address** your BME280 (0x66 or 0x67) in **display_manager_setting.h** file on line 11.
|
```bash
#define DISPLAY_BME_ADDRESS 0x76
```


7. *Edit:*change SDA&SCL PIN  in **display_manager_setting.h** file on line 13 & 14.

```bash
#define DISPLAY_BME_SDA_PIN 21
#define DISPLAY_BME_SCL_PIN 19
```
8. *Edit:*change time of autochange screen in **display_manager_setting.h** file on line 28.


```bash
#define SCREEN_AUTO_SWITCH_INTERVAL 60
```

9. That all. Sucess intergration to [Duino-Coin Miner File](https://github.com/revoxhere/duino-coin)

10. Upload the program via Arduino, adjusting the settings to your board


12. **If everything is connected correctly and the code is upload correctly, everything should work :)**

## 12. 3D Printed case & assembly
```bash
#FUTURE TO DO
```
## 13. Miner photos

![App Screenshot](https://placehold.co/800x350)

![App Screenshot](https://placehold.co/800x350)


## 14. To do: [future]
- At the moment everything is done as planned
- I might consider creating a Wi-Fi bride so that all miners can connect to one. However, I suspect the hash rate will drop significantly or there will be program lag.
- Sending the project to the DCUO project admins so they can post it on the github page in the community project if they like it :)

## 15. Links

 - [Official Duino-Coin webiste](https://duinocoin.com/)
 - [Duino-Coin Github (miner files download)](https://github.com/revoxhere/duino-coin)
 - [TFT_eSPI Libary Github](https://github.com/Bodmer/TFT_eSPI)
 - [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
 - [space.LAB](https://adamko.space/spacelab/)

## 16. Support

For support my work, send DUCO to "adamco"

or buy me a coffee on https://buycoffee.to/adamko.space.lab




