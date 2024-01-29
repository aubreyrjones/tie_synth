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
        k = k % wave.len;
        if (k < 0) k += wave.len;
    }
    else if (k >= wave.len) {
        k = k % wave.len;
    }

    return wave.t[k];
}

float sinc(float x) {
    return sin(x * PI) / (x * PI);
}

constexpr std::array blackman_window = {-1.3877787807814457e-17, 0.0664466094067262, 0.3399999999999999, 0.7735533905932737, 0.9999999999999999, 0.7735533905932739, 0.3400000000000001, 0.06644660940672628, -1.3877787807814457e-17};

float window(float m, int M) {
    if (m >= 0 && m <= M) {
        //return 0.42f - (0.5f * cosf((2 * PI * m) / M)) + (0.08f * cosf((4 * PI * m) / M));
        // could probably interpolate this, but leave it for now.
        return blackman_window[m];
    }

    return 0;
}

void windowed_sinc_interpolation(buffer input, buffer output, float inputSampleRate, float outputSampleRate, sample_func samplePolicy) {
    int windowSize = 8;
    int halfWindow = windowSize / 2;

    float sincScale = min(inputSampleRate, outputSampleRate) / inputSampleRate;
    float sampleRatio = inputSampleRate / outputSampleRate;

    for (int j = 0; j < output.len; j++) {
        float accum = 0;
        float J = j * sampleRatio;
        int kLow = ceilf(J - halfWindow);
        int kHigh = floorf(J + halfWindow);

        for (int k = kLow; k <= kHigh; k++) {
            accum += sinc(sincScale * (k - J)) * window(k - J + halfWindow, windowSize) * samplePolicy(k, input);
        }

        output.t[j] = min(1.f, outputSampleRate / inputSampleRate) * accum;
    }
}


void pitch_shift_looped(buffer loop, buffer stream, float nativeSampleRate, float originalPitch, float targetPitch) {
    float shiftedRate = nativeSampleRate * (originalPitch / targetPitch);
    windowed_sinc_interpolation(loop, stream, nativeSampleRate, shiftedRate, sample_loop);
}

void pitch_shift_single_cycle(buffer loop, buffer stream, float nativeSampleRate, float targetPitch) {
    float originalPitch = nativeSampleRate / loop.len;
    pitch_shift_looped(loop, stream, nativeSampleRate, originalPitch, targetPitch);
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

    resample::pitch_shift_single_cycle({signal.data(), signal_table_size}, {workingArray.data(), signal_table_size}, AUDIO_SAMPLE_RATE_EXACT, sampler.frequency);

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        block->data[i] = workingArray.data()[i] * 32000;
    }

    // for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    //     block->data[i] = sampler.sample() * 32000;
    //     sampler.step();
    // }

    transmit(block);
    release(block);
}