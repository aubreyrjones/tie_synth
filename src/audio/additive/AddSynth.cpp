#include "AddSynth.hpp"

namespace audio {

void AdditiveSynth::doSetup() {
    analyzer.loadFromSD("a.wav");
}

AdditiveSynth as_module;

}