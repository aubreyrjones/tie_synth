#include "display.h"

namespace display {


void draw_buffer_in_scope(float *data) {
    scope_oled.clearBuffer();
    for (int i = 0; i < 128; i++) {
        float s = data[i];
        u8g2_uint_t height = abs(s) * 64;
        if (s > 0) {
            scope_oled.drawPixel(i, 31 + height);
        } else {
            scope_oled.drawPixel(i, 31 - height);
        }
    }
    scope_oled.sendBuffer();
}

void draw_buffer_in_scope2(float *data) {
    scope_oled.clearBuffer();
    for (int i = 0; i < 128; i++) {
        float s = data[i];
        scope_oled.drawPixel(i, 32 + (32 * s));
    }
    scope_oled.sendBuffer();
}

// main display on SPI1 for menus and navigation.
Adafruit_SSD1351 main_oled = Adafruit_SSD1351(
    SCREEN_WIDTH, SCREEN_HEIGHT, &SPI1, CS_PIN, DC_PIN, RST_PIN); // for SPI1
// scope display
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI 
    scope_oled(U8G2_R0, SCOPE_CS_PIN, SCOPE_DC_PIN, SCOPE_RESET_PIN);

void initialize_oleds() {
  SPI1.setCS(CS_PIN);
  SPI1.setMISO(MISO_PIN); // move this off of the "normal" pin at the head of the board

  main_oled.begin(12000000);
  main_oled.cp437(true); // we're not legacy code, fix the character map bug
  main_oled.fillScreen(0);

  main_oled.setCursor(10, 10);
  main_oled.setTextColor(0);
  main_oled.setTextSize(3);

  scope_oled.setBusClock(10000000);
  scope_oled.begin();
  scope_oled.clearBuffer();
  scope_oled.setFont(u8g2_font_ncenB14_tr);
  scope_oled.drawStr(8, 24, "Greetings");
  scope_oled.drawStr(30, 60, "Operator!");
  scope_oled.sendBuffer();
  delay(1000);
}
} // namespace display