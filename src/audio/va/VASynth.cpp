#include "VASynth.hpp"

namespace audio {

void VASynth::doSetup() {

    va_osc_mixer.gain(2, 0); // there are no controls addressing these channels of the oscillator mixer.
    va_osc_mixer.gain(3, 0);

}


VASynth va_module;

}