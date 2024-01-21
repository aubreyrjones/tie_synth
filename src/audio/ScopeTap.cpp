#include "ScopeTap.h"

AudioAnalyzeScope::AudioAnalyzeScope() : AudioStream(1, inputQueueArray) {
    // any extra initialization
}


void AudioAnalyzeScope::update(void) {
    audio_block_t* input = receiveReadOnly(0);

    memcpy(lastFrame, input, sizeof(int16_t) * AUDIO_BLOCK_SAMPLES);

    release(input);
}