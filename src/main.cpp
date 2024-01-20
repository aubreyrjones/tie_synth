#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=591.3333129882812,292.3333435058594
AudioSynthWaveformSine   sine1;          //xy=902.3333129882812,366.33331298828125
AudioOutputI2S           i2s2;           //xy=1224.3333282470703,359.3333282470703
AudioConnection          patchCord1(sine1, 0, i2s2, 0);
AudioConnection          patchCord2(sine1, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=110.33332824707031,58.333335876464844
// GUItool: end automatically generated code


#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <U8g2lib.h>
#include <MIDI.h>

constexpr float hw_output_volume = 0.5f;


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

// Displays

// main display on SPI1 for menus and navigation.
Adafruit_SSD1351 main_oled = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI1, CS_PIN, DC_PIN, RST_PIN); // for SPI1
// scope display
U8G2_SSD1309_128X64_NONAME0_1_4W_HW_SPI scope_oled(U8G2_R0, 10, 17, 16);

struct FastMIDIBaud
{
    static const long BaudRate = 1000000;
};
//MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial7, serial_midi, FastMIDIBaud);

MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud> serialMIDI(Serial7);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud>> serial_midi(serialMIDI);

void setup() {
  AudioMemory(12);

  Serial.begin(38400); // terminal with computer
  //serial_midi.begin(); // so if I call this, it doesn't work
  Serial7.begin(FastMIDIBaud::BaudRate); // but if I call this, it does...

  
  // Set up sound chips.
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.volume(hw_output_volume);

  sine1.frequency(440);
  sine1.amplitude(0.5f);

  SPI1.setCS(CS_PIN);
  SPI1.setMISO(39); // move this off of the "normal" pin at the head of the board

  main_oled.begin();
  main_oled.cp437(true); // we're not legacy code, fix the character map bug
  main_oled.fillScreen(WHITE);

  main_oled.setCursor(10, 10);
  main_oled.setTextColor(0);
  main_oled.print("yo! \xba");

  scope_oled.begin();
}

void loop() {
  if (serial_midi.read()) {
    main_oled.setCursor(10, 10);
    main_oled.setTextColor(0, WHITE);
    main_oled.print(serial_midi.getData1(), DEC);
    main_oled.print(" ");
    main_oled.print(serial_midi.getData2(), DEC);
  }

  scope_oled.firstPage();
  do {
    scope_oled.setFont(u8g2_font_ncenB14_tr);
    scope_oled.drawStr(8, 24, "Greetings");
    scope_oled.drawStr(30, 60, "Operator!");
  } while (scope_oled.nextPage());
}
