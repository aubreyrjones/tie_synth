#include "midi_impl.hpp"
#include "audio/additive/AddSynth.hpp"

namespace midi_impl {

void recvNoteOn(byte channel, byte note, byte velocity) {
    audio::as_module.noteOn(note, velocity / 127.f);
}

void recvNoteOff(byte channel, byte note, byte velocity) {

}

void recvControlChange(byte channel, byte control, byte value) {

}

// void recvAfterTouchPoly(byte channel, byte note, byte velocity);
// void recvProgramChange(byte channel, byte program);
// void recvAfterTouch(byte channel, byte pressure);
// void recvPitchChange(byte channel, int pitch);
// void recvSystemExclusiveChunk(const byte *data, uint16_t length, bool last);
// void recvSystemExclusive(byte *data, unsigned int length);
// void recvTimeCodeQuarterFrame(byte data);
// void recvSongPosition(uint16_t beats);
// void recvSongSelect(byte songNumber);
// void recvTuneRequest();
// void recvClock();
// void recvStart();
// void recvContinue();
// void recvStop();
// void recvActiveSensing();
// void recvSystemReset();
// void recvRealTimeSystem(byte realtimebyte);

}