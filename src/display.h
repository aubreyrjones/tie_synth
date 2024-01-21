#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <U8g2lib.h>

namespace display {
// Screen dimension
constexpr byte SCREEN_WIDTH = 128;
constexpr byte SCREEN_HEIGHT = 128;

// Pins
constexpr byte CS_PIN = 38; // for CS1: 38
constexpr byte DC_PIN = 30;
constexpr byte RST_PIN = 31;
constexpr byte DIN_PIN = 26; // for MOSI1: 26
constexpr byte CLK_PIN = 27; // for SCK1: 27

extern Adafruit_SSD1351 main_oled;
extern U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI 
//extern U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI 
    scope_oled;

void initialize_oleds();

}