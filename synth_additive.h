#ifndef synth_additive_h_
#define synth_additive_h_


#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    // github.com/PaulStoffregen/cores/blob/master/teensy4/arm_math.h
#include <array>

class AudioSynthAdditive : public AudioStream {
public:
private:
    /// @brief If this is throwing up problems, you've set the `partial_table_size` to an unsupported value.
    template<bool flag = false> void static_no_match() { static_assert(flag, "FFT size not supported."); }

public:
    static constexpr auto partial_table_size = 4096;
    static constexpr auto signal_table_size = partial_table_size;
    std::array<float, partial_table_size> partialTable; // frequency domain, packed amplitude and phase
    std::array<float, signal_table_size> signal; // time domain, amplitude

    arm_rfft_fast_instance_f32 fftInstance;

    /// @brief Phase tracker for spectral mode.
    int rawPlaybackPhase = 0;
    
    bool doDebug = false;

public:

    AudioSynthAdditive(void) : AudioStream(0, NULL) {
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

        partialTable[2] = 40; // add something to the fundamental
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
    AudioSynthOscBank(void) : AudioStream(0, NULL) { }

    virtual void update(void) override;
};

#endif

