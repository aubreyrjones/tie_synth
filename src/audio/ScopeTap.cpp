#include "ScopeTap.h"

AudioAnalyzeScope::AudioAnalyzeScope() : AudioStream(1, inputQueueArray) {
    // any extra initialization
}


void AudioAnalyzeScope::update(void) {
    audio_block_t* input = receiveReadOnly(0);
    if (!input) return;

    //memcpy(lastFrame, input, sizeof(int16_t) * AUDIO_BLOCK_SAMPLES);
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        sampleBuffer.push(input->data[i]);
    }

    release(input);
}