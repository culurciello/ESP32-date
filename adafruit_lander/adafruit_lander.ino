
// inspired by: https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <WiFi.h>
#include "time.h"
#include <Adafruit_AHTX0.h>
#include <Adafruit_NeoPixel.h>

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
Adafruit_AHTX0 aht; // sensors board: Adafruit AHT20 - Temperature & Humidity Sensor Breakout Board
Adafruit_NeoPixel np = Adafruit_NeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

int led = LED_BUILTIN;

const char* ssid       = "your-ssid-name";
const char* password   = "your-ssid-password";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000; // EST time
const int   daylightOffset_sec = 3600;

void printScreen()
{
  struct tm timeinfo;  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // %A	returns day of week
  // %B	returns month of year
  // %d	returns day of month
  // %Y	returns year
  // %H	returns hour
  // %M	returns minutes
  // %S	returns seconds

  // read sensors:
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

  // display time:
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println();
  display.println(&timeinfo, "%a %b %d");
  display.println(&timeinfo, "%Y");
  display.println();
  display.setTextSize(4);
  display.println(&timeinfo, "%H");
  display.println(&timeinfo, "%M");
  display.setTextSize(1);
  display.println();
  display.print("T "); display.print(temp.temperature); display.println(" C");
  display.print("H "); display.print(humidity.relative_humidity); display.println(" %");
  display.display(); // actually display all of the above
}


void setup() {
  Serial.begin(115200);
  Serial.println("128x64 OLED clock by Euge");
  delay(500); // wait for the OLED to power up

  // init display:
  display.begin(0x3C, true); // Address 0x3C default
  display.clearDisplay();
  display.display();
  display.setRotation(0);
  display.setTextColor(SH110X_WHITE);

  // init sensors: 0x38
  if (! aht.begin(&Wire, 0, 0x38)) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

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

void loop() {
  for (int inp=0; inp<60; inp++) {
    np.setPixelColor(0, 255-4.25*inp, inp, 4.25*inp);
    digitalWrite(led, HIGH);
    np.show(); 
    delay(500);
    digitalWrite(led, LOW);
    np.clear();
    delay(500);
    printScreen(); // once per second
  }
}
