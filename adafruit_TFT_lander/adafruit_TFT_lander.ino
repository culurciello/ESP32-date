
// inspired by: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <WiFi.h>
#include "time.h"
#include <Adafruit_NeoPixel.h>
#include <Fonts/FreeSans12pt7b.h>

Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_NeoPixel np = Adafruit_NeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

GFXcanvas16 canvas(135, 240);

int led = LED_BUILTIN;

const char* ssid       = "your-ssid-name";
const char* password   = "your-ssid-password";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  // EST time
const int daylightOffset_sec = 3600;

void printScreen() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // %A returns day of week
  // %B returns month of year
  // %d returns day of month
  // %Y returns year
  // %H returns hour
  // %M returns minutes
  // %S returns seconds

  // display time:
  canvas.fillScreen(ST77XX_BLACK);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(0, 20);
  canvas.println(&timeinfo, "%A");
  canvas.setTextColor(ST77XX_MAGENTA);
  canvas.println(&timeinfo, "  %b %d");
  canvas.setTextColor(ST77XX_BLUE);
  canvas.println(&timeinfo, "    %Y");
  canvas.println();
  canvas.setTextColor(ST77XX_YELLOW);
  canvas.setTextSize(3);
  canvas.setCursor(25, 155);
  canvas.println(&timeinfo, "%H");
  canvas.setTextColor(ST77XX_GREEN);
  canvas.setCursor(25, 215);
  canvas.println(&timeinfo, "%M");
  display.drawRGBBitmap(0, 0, canvas.getBuffer(), 135, 240);
}


void setup() {
  Serial.begin(115200);
  Serial.println("240x135 TFT clock by Euge");
  delay(500);  // wait for the OLED to power up

  // init display:
  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);
  // initialize TFT
  display.init(135, 240);  // Init ST7789 240x135
  display.setRotation(2);
  // canvas.setTextWrap(false);
  canvas.setFont(&FreeSans12pt7b);
  canvas.setTextColor(ST77XX_WHITE);
  Serial.println(F("Initialized"));

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printScreen();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // LEDs
  np.begin();
  np.clear();
  np.setBrightness(20);
  np.show();
  pinMode(led, OUTPUT);
}


uint8_t * myWheel(byte WheelPos) {
  static uint8_t colors[3];
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    colors[0]=255 - WheelPos * 3;
    colors[1]=0;
    colors[2]=WheelPos * 3;
    return colors;//{255 - WheelPos * 3, 0, WheelPos * 3};
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    colors[0]=0;
    colors[1]=WheelPos * 3;
    colors[2]=255 - WheelPos * 3;
    return colors;//{0, WheelPos * 3, 255 - WheelPos * 3};
  }
  WheelPos -= 170;
  colors[0]=WheelPos * 3;
  colors[1]=255 - WheelPos * 3;
  colors[2]=0;
  return colors;//{WheelPos * 3, 255 - WheelPos * 3, 0};
}

boolean ledOn = LOW;
uint8_t j = 0; // neopixel color wheel

void loop() {
  for (int inp = 0; inp < 60; inp++) {
    uint8_t * colors = myWheel(j++);
    np.setPixelColor(0, colors[0], colors[1], colors[2]);
    np.show();
    ledOn = !ledOn;
    digitalWrite(led, ledOn);
    delay(1000);
    printScreen();  // once per second
  }
}

