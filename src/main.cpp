#include "audio/gui_gen.icc"
#include "audio/ScopeTap.h"

AudioAnalyzeScope scopeTap;

AudioConnection patchCord_0(sine1, 0, scopeTap, 0);

#include <Metro.h>
#include <MIDI.h>
#include "display.h"

constexpr float hw_output_volume = 0.5f;

// Color
const uint16_t WHITE = 0xFFFF;

// Displays

struct FastMIDIBaud
{
    static const long BaudRate = 1000000;
};

MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud> _midi_transport(Serial7);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud>> serial_midi(_midi_transport);

Metro scopeRepaint(16);

float curFreq = 440.f;
float curAmp = 1.f;

void setup() {
  AudioMemory(12);
  randomSeed(analogRead(0));

  Serial.begin(38400); // terminal with computer
  serial_midi.begin(MIDI_CHANNEL_OMNI); // listen on all channels, we'll sort it out ourselves.
  
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
    }
    else {
      main_oled.print("-");
    }

    if (serial_midi.getData1() == 1) {
      if (serial_midi.getData2() > 50) {
        sine1.frequency(curFreq += 20.f);
      }
      else {
        sine1.frequency(curFreq -= 20.f);
      }
    }

    if (serial_midi.getData1() == 2) {
      if (serial_midi.getData2() > 50) {
        sine1.amplitude(curAmp += 0.05f);
      }
      else {
        sine1.amplitude(curAmp -= 0.05f);
      }
    }
  }

  if (scopeRepaint.check()) {
  scope_oled.clearBuffer();
  for (int i = 0; i < 128; i++) {
    float s = scopeTap.lastFrame[i] / 65536.f + 0.5f;
    u8g2_uint_t height = s * 64;
    scope_oled.drawVLine(i, 0, height);
  }
  scope_oled.sendBuffer();
  scopeRepaint.reset();
  }
  
}
