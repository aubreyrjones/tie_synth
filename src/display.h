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

// Pins for main color OLED
// On SPI1
constexpr byte CS_PIN = 38;
constexpr byte MISO_PIN = 39;
constexpr byte DC_PIN = 30;
constexpr byte RST_PIN = 31;
constexpr byte DIN_PIN = 26; 
constexpr byte CLK_PIN = 27; 

// Pins for scope OLED
// on SPI (0)
constexpr byte SCOPE_CS_PIN = 10;
constexpr byte SCOPE_DC_PIN = 17;
constexpr byte SCOPE_RESET_PIN = 16; 

extern Adafruit_SSD1351 main_oled;
extern U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI 
//extern U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI 
    scope_oled;

void initialize_oleds();

void draw_buffer_in_scope(float *data);
void draw_buffer_in_scope2(float *data);

}