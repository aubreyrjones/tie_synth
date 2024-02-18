#include "AddSynth.hpp"

namespace audio {

void AdditiveSynth::doSetup() {
    analyzer.loadFromSD("a.wav");
}

void AdditiveSynth::noteOn(NoteNumber note, float velocity){
    frequency.set(NoteFreqs[note]);
}

void AdditiveSynth::noteOff(NoteNumber note, float velocity){

}

void AdditiveSynth::controlChange(CCNumber cc, byte value){

}

AdditiveSynth as_module;

}