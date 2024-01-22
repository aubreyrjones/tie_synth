#include "audio/ScopeTap.h"
#include "audio/gui_gen.icc"

AudioAnalyzeScope scopeTap;

AudioConnection patchCord_0(sine1, 0, scopeTap, 0);

#include "display.h"
#include <MIDI.h>
#include <Metro.h>
#include "gui/screen.hpp"

constexpr float hw_output_volume = 0.5f;

// Color
const uint16_t WHITE = 0xFFFF;

// Displays

struct FastMIDIBaud {
  static const long BaudRate = 1000000;
};

MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud> _midi_transport(Serial7);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud>> serial_midi(_midi_transport);

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

gui::WidgetInput cc_to_input_event(midi::DataByte const& cc, midi::DataByte const& val) {
  if (cc == 1) {
    if (val) {
      return gui::WidgetInput::RIGHT_INCR;
    }
    else {
      return gui::WidgetInput::RIGHT_DECR;
    }
  }

  if (cc == 2) {
    if (val) {
      return gui::WidgetInput::LEFT_INCR;
    }
    else {
      return gui::WidgetInput::LEFT_DECR;
    }
  }

  if (cc == 8) {
    if (val) {
      return gui::WidgetInput::RIGHT_PUSH;
    }
    else {
      return gui::WidgetInput::RIGHT_REL;
    }
  }

  if (cc == 9) {
    if (val) {
      return gui::WidgetInput::LEFT_PUSH;
    }
    else {
      return gui::WidgetInput::LEFT_REL;
    }
  }

  return gui::WidgetInput::NULL_INPUT;
}


Metro blankTimeout(60000); // 1 minute screen sleep timer
Metro scopeRepaint(40); // 25fps

bool blankMode = false;

void loop() {
  using namespace display;

  bool has_midi_input = serial_midi.read();
  if (has_midi_input) {
    blankTimeout.reset();
    blankMode = false;
  }

  if (blankTimeout.check() || blankMode) {
    blankMode = true;
    gui::activeScreen->sully(); // so it'll repaint next time.
    main_oled.fillScreen(0);
    scope_oled.clearDisplay();
    return;
  }

  if (has_midi_input && serial_midi.getChannel() == 16) {
    auto cc = serial_midi.getData1();
    auto signal = serial_midi.getData2();
    
    if (cc == 0) {
      if (signal) { // positive increment
        gui::activeScreen->nextWidget();
      }
      else {
        gui::activeScreen->prevWidget();
      }
    }
    else if (cc == 1 || cc == 2 || cc == 8 || cc == 9) {
      gui::activeScreen->handleInput(cc_to_input_event(cc, signal));
    }
  }

  gui::activeScreen->draw();


  if (scopeRepaint.check()) {
    scopeRepaint.reset();
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
  }
}
