#pragma once

#include <Arduino.h>
#include "../ext/Audio/Audio.h"
#include <CircularBuffer.hpp>

class AudioAnalyzeScope : public AudioStream
{
public:
    AudioAnalyzeScope();
    virtual void update(void);

    //int16_t lastFrame[AUDIO_BLOCK_SAMPLES];
    CircularBuffer<int16_t, 128> sampleBuffer {};

private:
    audio_block_t *inputQueueArray[1];
};