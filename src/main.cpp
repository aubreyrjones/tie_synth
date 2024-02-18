#include "audio/ScopeTap.h"
#include "audio/gui_gen.icc"

AudioAnalyzeScope scopeTap;
AudioConnection patchCord_0(output_mixer, 0, scopeTap, 0);

#include "display.h"
#include <MIDI.h>
#include <Metro.h>
#include "gui/screen.hpp"
#include "audio/Control.hpp"
#include "audio/va/VASynth.hpp"
#include "audio/additive/AddSynth.hpp"
#include "midi_impl.hpp"

// Teensy-provided function to get the chip temperature.
extern float tempmonGetTemp(void);

constexpr float hw_output_volume = 0.5f;
constexpr size_t n_audio_blocks_allocated = 64;

struct FastMIDIBaud {
  static const long BaudRate = 1000000;
};

MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud> _midi_transport(Serial7);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, FastMIDIBaud>> serial_midi(_midi_transport);

void setup() {
  // adjust the clock speed for the PSRAM mapped to EXTMEM
  CCM_CBCMR &= ~(CCM_CBCMR_FLEXSPI2_PODF_MASK | CCM_CBCMR_FLEXSPI2_CLK_SEL_MASK); // clear settings
  CCM_CBCMR |= (CCM_CBCMR_FLEXSPI2_PODF(3) | CCM_CBCMR_FLEXSPI2_CLK_SEL(3)); // 132 MHz

  AudioMemory(n_audio_blocks_allocated);
  randomSeed(analogRead(0));

  Serial.begin(38400);                  // terminal with computer

  Serial.print("Startup.");

  usbMIDI.begin();
  usbMIDI.setHandleNoteOn(midi_impl::recvNoteOn);
  usbMIDI.setHandleNoteOff(midi_impl::recvNoteOff);

  serial_midi.begin(MIDI_CHANNEL_OMNI); // listen on all channels, we'll sort it out ourselves.

  // Set up sound chips.
  sgtl5000_1.setAddress(LOW);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.volume(hw_output_volume);

  display::initialize_oleds();

  // initialize the modules
  audio::va_module.doSetup();
  audio::as_module.doSetup();

  Serial.println("Finished synth setup.\nInitializing controls.");

  // run all controls to initialize them.
  audio::run_all_control_updates();

}


void draw_default_scope() {
  using namespace display;
  scope_oled.clearBuffer();
  for (int i = 0; i < scopeTap.sampleBuffer.size(); i++) {
    float s = scopeTap.sampleBuffer[i] / 65536.f;
    //float s = additive1.samples()[i];
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


Metro blankTimeout(60000); // 1 minute screen sleep timer
Metro scopeRepaint(40); // 25fps
Metro perfRepaint(100);

bool blankMode = false;

void loop() {
  using namespace display;

  usbMIDI.read();
  bool has_midi_input = serial_midi.read();

  if (has_midi_input) {
    blankTimeout.reset();
    blankMode = false;
  }

  if (blankMode) {
    return;
  }

  if (blankTimeout.check()) {
    blankMode = true;
    gui::activeScreen->sully(); // so it'll repaint next time.
    main_oled.fillScreen(0);
    scope_oled.clearDisplay();
    return;
  }

  if (has_midi_input && serial_midi.getChannel() == 16) {
    auto cc = serial_midi.getData1();
    auto signal = serial_midi.getData2();
    
    gui::handleUserInput(cc, signal);

  }

  if (scopeRepaint.check()) {
    scopeRepaint.reset();
    
    if (gui::activeScreen->hasScope()) {
      gui::activeScreen->drawScope();
    }
    else {
      draw_default_scope();
    }
  }

  gui::activeScreen->draw();

  if (perfRepaint.check()){
    perfRepaint.reset();
    if (!gui::activeScreen->showPerf()) return;

    main_oled.drawFastHLine(0, 128 - 10, 128, colors::white);
    main_oled.setTextSize(1);
    main_oled.setCursor(0, 128 - 8);
    main_oled.setTextColor(colors::white, 0);
    main_oled.print("D:");
    main_oled.print(AudioProcessorUsage(), 0);
    main_oled.print("% ");

    main_oled.print(tempmonGetTemp(), 0);
    main_oled.print("C");

    main_oled.setCursor(64, 128 - 8);
    main_oled.print("M:");
    main_oled.print(AudioMemoryUsage());
    main_oled.print("(");
    main_oled.print(AudioMemoryUsageMax());
    main_oled.print(")");
  }
}
