#include "VASynth.hpp"

namespace audio {

void VASynth::doSetup() {
    float shapeFunc[] = {-1.f, 1.f};
    va_waveshape.shape(shapeFunc, 2);

    va_osc3.frequencyModulation(3);

    va_osc_mixer.gain(3, 0);

}


VASynth va_module;

}