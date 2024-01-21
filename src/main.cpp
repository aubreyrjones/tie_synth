#include "audio/ScopeTap.h"
#include "audio/gui_gen.icc"

AudioAnalyzeScope scopeTap;

AudioConnection patchCord_0(sine1, 0, scopeTap, 0);

#include "display.h"
#include <MIDI.h>
#include <Metro.h>

constexpr float hw_output_volume = 0.5f;

// Color
const uint16_t WHITE = 0xFFFF;

// Displays

struct FastMIDIBaud {
  static const long BaudRate = 1000000;
};

MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud> _midi_transport(Serial7);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud>> serial_midi(_midi_transport);

Metro scopeRepaint(33);

float curFreq = 440.f;
float curAmp = 1.f;

void setup() {
  AudioMemory(12);
  randomSeed(analogRead(0));

  Serial.begin(38400);                  // terminal with computer
  serial_midi.begin(MIDI_CHANNEL_OMNI); // listen on all channels, we'll sort it
                                        // out ourselves.

  // Set up sound chips.
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  sgtl5000_1.volume(hw_output_volume);

  sine1.frequency(curFreq);
  sine1.amplitude(curAmp);

  display::initialize_oleds();
}

void loop() {
  using namespace display;
  if (serial_midi.read()) {
    main_oled.setCursor(40, 50);
    main_oled.setTextColor(WHITE, 0);
    main_oled.print(serial_midi.getData1(), DEC);
    main_oled.print(" ");
    if (serial_midi.getData2()) {
      main_oled.print("+");
    } else {
      main_oled.print("-");
    }

    if (serial_midi.getData1() == 1) {
      auto incr = serial_midi.getData2() > 50 ? 20.f : -20.f;
      curFreq += incr;
      if (curFreq <= 0) {
        curFreq = 2;
      }
      sine1.frequency(curFreq);
    }

    if (serial_midi.getData1() == 2) {
      auto incr = serial_midi.getData2() > 50 ? 0.05f : -0.05f;
      curAmp += incr;
      if (curAmp <= 0.05f) {
        curAmp = 0.05f;
      }
      else if (curAmp > 1.f) {
        curAmp = 1.f;
      }
      sine1.amplitude(curAmp);
    }
  }

  if (scopeRepaint.check()) {
    scope_oled.clearBuffer();
    for (int i = 0; i < 128; i++) {
      float s = scopeTap.lastFrame[i] / 65536.f;
      u8g2_uint_t height = abs(s) * 64;
      if (s > 0) {
        // scope_oled.drawVLine(i, 31, height);
        scope_oled.drawPixel(i, 31 + height);
      } else {
        // scope_oled.drawVLine(i, 31 - height, height);
        scope_oled.drawPixel(i, 31 - height);
      }
    }
    scope_oled.sendBuffer();
    scopeRepaint.reset();
  }
}
