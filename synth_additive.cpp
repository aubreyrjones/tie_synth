#include "synth_additive.h"

void AudioSynthAdditive::update() {
    auto block = allocate();
    if (!block) return;

    // super naive additive algorithm follows for now, lol

    // calculate the waveform for this frame
    arm_rfft_fast_f32(&fftInstance, partialTable.data(), signal.data(), true);
    
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        sampler.step();
        block->data[i] = sampler.sample() * 32000;
    }

    transmit(block);
    release(block);
}