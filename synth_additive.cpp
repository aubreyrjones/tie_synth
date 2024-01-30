#include "synth_additive.h"

namespace resample {

/// @brief A pointer to a signal buffer and its length.
struct buffer {
    float *t;
    int len;
};

/// @brief Sample function definition.
using sample_func = float (*)(int, buffer);


float sample_oneshot(int i, buffer wave) {
    if (i < 0 || i >= wave.len) {
        return 0;
    }

    return wave.t[i];
}

float sample_loop(int k, buffer wave) {
    if (k < 0) {
        while (k < 0) k += wave.len;
    }
    else if (k >= wave.len) {
        if (wave.len == 256) {
            k = k & 0xff;
        }
        else {
            k = k % wave.len;
        }
    }

    return wave.t[k];
}

float sinc(float x) {
    if (abs(x) <= std::numeric_limits<float>::epsilon()) return 1;
    return arm_sin_f32(x * PI) / (x * PI);
}

float window(float m, int M, bool debugFlag) {
    if (m >= 0 && m <= M) {
        return 0.42f - (0.5f * arm_cos_f32((2 * PI * m) / M)) + (0.08f * arm_cos_f32((4 * PI * m) / M));
    }

    return 0;
}

// float mod(int x, int m) {
//     return x % m;
// }

float mod(float x, int m) {
    float intPart;
    float fracPart = modf(x, &intPart);
    return ((int) intPart % m) + fracPart;
}

void windowed_sinc_interpolation(buffer input, buffer output, float inputSampleRate, float outputSampleRate, sample_func samplePolicy, int phase, bool debugFlag) {
    int windowSize = 8;
    int halfWindow = windowSize / 2;

    float sincScale = min(inputSampleRate, outputSampleRate) / inputSampleRate;
    float sampleRatio = inputSampleRate / outputSampleRate;

    for (int j = 0; j < output.len; j++) {
        float accum = 0;

        float J = (j + phase) * sampleRatio;
        int kLow = floorf(J - halfWindow);
        int kHigh = ceilf(J + halfWindow);
        float intPart;

        std::array<float, 9> kMinusJ {};
        for (int k = kLow, ki = 0, s = -halfWindow; k <= (kLow + windowSize); k++, ki++, s++) {
            kMinusJ[ki] = s - modf(J, &intPart);
        }

        std::array<float, 9> sincTable {};
        for (int ki = 0; ki <= windowSize; ki++) {
            sincTable[ki] = sinc(sincScale * kMinusJ[ki]) * window(kMinusJ[ki] + halfWindow, windowSize, debugFlag);
        }

        for (int ki = 0, k = kLow; ki <= windowSize; ki++, k++) {
            accum += sincTable[ki] * samplePolicy(k, input);
        }

        // for (int k = kLow; k <= kHigh; k++, ki++) {
        //     accum += sinc(sincScale * (k - J)) * window(k - J + halfWindow, windowSize, debugFlag) * samplePolicy(ki, input);
        // }

        output.t[j] = min(1.f, outputSampleRate / inputSampleRate) * accum;
    }
}


void pitch_shift_looped(buffer loop, buffer stream, float nativeSampleRate, float originalPitch, float targetPitch, int phase, bool debugFlag) {
    float shiftedRate = nativeSampleRate * (originalPitch / targetPitch);
    windowed_sinc_interpolation(loop, stream, nativeSampleRate, shiftedRate, sample_loop, phase, debugFlag);
}

void pitch_shift_single_cycle(buffer loop, buffer stream, float nativeSampleRate, float targetPitch, int phase, bool debugFlag) {
    float originalPitch = nativeSampleRate / loop.len;
    pitch_shift_looped(loop, stream, nativeSampleRate, originalPitch, targetPitch, phase, debugFlag);
}


} //namespace resample

void AudioSynthAdditive::update() {
    auto block = allocate();
    if (!block) return;

    // super naive additive algorithm follows for now, lol

    decltype(partialTable) workingArray;
    memcpy(workingArray.data(), partialTable.data(), sizeof(float) * partialTable.size());

    // calculate the waveform for this frame
    arm_rfft_fast_f32(&fftInstance, workingArray.data(), signal.data(), 1);

    resample::pitch_shift_single_cycle({signal.data(), signal_table_size}, {workingArray.data(), AUDIO_BLOCK_SAMPLES}, AUDIO_SAMPLE_RATE_EXACT, sampler.frequency, sampleIndex, useWindow);

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        block->data[i] = workingArray.data()[i] * 32000;
    }
    sampleIndex += AUDIO_BLOCK_SAMPLES;

    // for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    //     block->data[i] = sampler.sample() * 32000;
    //     sampler.step();
    // }

    transmit(block);
    release(block);
}