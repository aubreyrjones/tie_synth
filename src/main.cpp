#include <Arduino.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

// Screen dimension
const byte SCREEN_WIDTH = 128;
const byte SCREEN_HEIGHT = 128;

// Pins
const byte CS_PIN = 38; // for CS1: 38
const byte DC_PIN = 30;
const byte RST_PIN = 31;
const byte DIN_PIN = 26; // for MOSI1: 26
const byte CLK_PIN = 27; // for SCK1: 27

// Color
const uint16_t WHITE = 0xFFFF;

// Display
Adafruit_SSD1351 main_oled = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI1, CS_PIN, DC_PIN, RST_PIN); // for SPI1

void setup() {
  Serial.begin(38400);

  SPI1.setCS(CS_PIN);
  SPI1.setMISO(39); // move this off of RX/TX1

  Serial.println("Display Startup...!");
  main_oled.begin();
  main_oled.fillScreen(WHITE);
  Serial.println("Display Ready!");
}

void loop() {
  
}
