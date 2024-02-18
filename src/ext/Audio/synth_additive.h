#ifndef synth_additive_h_
#define synth_additive_h_


#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    
#include <array>

/// @brief What's the Nyquist frequency for the environment?
static constexpr auto systemNyquistFrequency = AUDIO_SAMPLE_RATE_EXACT / 2.f;

class AudioSynthAdditive : public AudioStream {
public:
private:
    /// @brief If this is throwing up problems, you've set the `partial_table_size` to an unsupported value.
    template<bool flag = false> void static_no_match() { static_assert(flag, "FFT size not supported."); }

public:
    static constexpr auto partial_table_size = 4096;
    static constexpr auto signal_table_size = partial_table_size;
    static constexpr float fundamental_frequency = AUDIO_SAMPLE_RATE_EXACT / signal_table_size;

    std::array<float, partial_table_size> partialTable; // frequency domain, packed amplitude and phase
    std::array<float, signal_table_size> signal; // time domain, amplitude

    arm_rfft_fast_instance_f32 fftInstance;

    /// @brief Phase tracker for spectral mode.
    int rawPlaybackPhase = 0;
    
    bool doDebug = false;

public:

    AudioSynthAdditive(void) : AudioStream(0, nullptr) {
        // The larger FFT tables get really big. By making this `if constexpr`, it ensures
        // we only ever reference the right-sized function. This means we only pull in the
        // tables we actually need.
        if constexpr (partial_table_size == 256) {
            arm_rfft_fast_init_256_f32(&fftInstance);
        }
        else if constexpr (partial_table_size == 1024) {
            arm_rfft_fast_init_1024_f32(&fftInstance);
        }
        else if constexpr (partial_table_size == 2048) {
            arm_rfft_fast_init_2048_f32(&fftInstance);
        }
        else if constexpr (partial_table_size == 4096) {
            arm_rfft_fast_init_4096_f32(&fftInstance);
        }
        else {
            static_no_match();
        }
    }

    /// @brief Get a reference to the partial array.
    /// @return A mutable reference to the partial array.
    std::array<float, partial_table_size>& partials() { return partialTable; }

    /// @brief Get a reference to the signal array.
    /// @return A mutable reference to the signal array.
    std::array<float, signal_table_size>& samples() { return signal; }

    void debug(bool debug) { doDebug = debug; }

    /// @brief Clear the whole partial table back to 0.
    void clearPartials();

    virtual void update(void) override;
};


class AudioSynthOscBank : public AudioStream {
public:
    static constexpr auto nBanks = 4;
    static constexpr auto bankSize = 16;

    struct VoicePrototype {
        std::array<float, bankSize> amplitudes {};
        std::array<uint32_t, bankSize> phaseOffsets {};
    };

protected:
    struct Bank {
        float fundamental = 172;
        std::array<uint32_t, bankSize> accumulators {};
        std::array<uint32_t, bankSize> phaseIncrements {};
        int cutoff = 0;
        bool active = false;

        void update();
        float sample();
        void frequency(float f);

        Bank() {
            std::fill(accumulators.begin(), accumulators.end(), 0);
            std::fill(phaseIncrements.begin(), phaseIncrements.end(), 0);
        }
    };

    /// @brief Sample the given bank.
    /// @param b reference to the bank to sample
    /// @return denormalized floating point signal
    float sample(Bank &b);

    /// @brief The oscillator banks.
    std::array<Bank, nBanks> banks {};

    /// @brief The voice profile we're playing.
    VoicePrototype voice {};

    bool _debug = false;

public:
    AudioSynthOscBank(void) : AudioStream(0, NULL) { 
        for (int i = 0; i < nBanks; i++) {
            frequency(i, 440);
        }

        voice.amplitudes[0] = 0.5f;
    }

    void frequency(int bank, float f);

    void setActive(int bank, bool active) { banks[bank].active = active; }

    void debug(bool d) { _debug = d; }

    VoicePrototype& getVoice() { return voice; }

    void previewVoice(float *out, int nSamples);

    virtual void update(void) override;
};


// class AudioSynthIFFTBank : public AudioStream {
// public:
//     /// @brief How many audible partials will each voice support, maximally?
//     static constexpr auto nPartials = 16;

//     /// @brief How long is the FFT buffer (at least twice the number of partials)
//     static constexpr auto fftBufferLength = std::max(256, nPartials * 2);

//     using FFTBuffer = std::array<float, fftBufferLength>;

//     /// @brief What's the Nyquist frequency for the environment?
//     static constexpr auto nyquistFreq = AUDIO_SAMPLE_RATE_EXACT / 2.f;

//     /// @brief How many simultaneous voices?
//     static constexpr auto nVoices = 1;

//     struct VoiceState {
//         FFTBuffer partials;
//         FFTBuffer signalA, signalB;
//         FFTBuffer *front = &signalA, *back = &signalB;

//         void swap() { std::swap(front, back); }
//     };

// protected:

//     /// @brief State for each voice.
//     std::array<VoiceState, nVoices> voices {};

//     /// @brief CMSIS FFT structure.
//     arm_rfft_fast_instance_f32 fftInstance;

// public:

//     AudioSynthIFFTBank();
// };

#endif

