#pragma once

#include <Arduino.h>
#include "../ext/Audio/Audio.h"

class AudioAnalyzeScope : public AudioStream
{
public:
    AudioAnalyzeScope();
    virtual void update(void);

    int16_t lastFrame[AUDIO_BLOCK_SAMPLES];

private:
    audio_block_t *inputQueueArray[1];
};