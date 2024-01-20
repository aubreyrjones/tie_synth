#include "display.h"

namespace display {

// main display on SPI1 for menus and navigation.
Adafruit_SSD1351 main_oled = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI1, CS_PIN, DC_PIN, RST_PIN); // for SPI1
// scope display
U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI scope_oled(U8G2_MIRROR_VERTICAL, 10, 17, 16);


void initialize_oleds() {
  SPI1.setCS(CS_PIN);
  SPI1.setMISO(39); // move this off of the "normal" pin at the head of the board

  main_oled.begin();
  main_oled.cp437(true); // we're not legacy code, fix the character map bug
  main_oled.fillScreen(0);

  main_oled.setCursor(10, 10);
  main_oled.setTextColor(0);
  main_oled.setTextSize(3);

  scope_oled.begin();

  scope_oled.clearBuffer();
  scope_oled.setFont(u8g2_font_ncenB14_tr);
  scope_oled.drawStr(8, 24, "Greetings");
  scope_oled.drawStr(30, 60, "Operator!");
  scope_oled.sendBuffer();
}
}