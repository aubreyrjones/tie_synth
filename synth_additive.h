#ifndef synth_additive_h_
#define synth_additive_h_


#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    // github.com/PaulStoffregen/cores/blob/master/teensy4/arm_math.h
#include <array>

class AudioSynthAdditive : public AudioStream {
protected:
    static constexpr auto partial_table_size = 256;
    static constexpr auto signal_table_size = partial_table_size;
    std::array<float, partial_table_size> partialTable; // frequency domain, packed amplitude and phase
    std::array<float, signal_table_size> signal; // time domain, amplitude

    arm_rfft_fast_instance_f32 fftInstance;

    float _frequency = 440;

    float playbackPhase = 0;
    bool doDebug = false;

public:
    AudioSynthAdditive(void) : AudioStream(0, NULL) {
        arm_rfft_fast_init_256_f32(&fftInstance);

        partialTable[2] = 40;
    }

    /// @brief Get a reference to the partial array.
    /// @return A mutable reference to the partial array.
    std::array<float, partial_table_size>& partials() { return partialTable; }

    /// @brief Get a reference to the signal array.
    /// @return A mutable reference to the signal array.
    std::array<float, signal_table_size>& samples() { return signal; }

    /// @brief Set the fundamental pitch for fundamental mode.
    /// @param freq 
    void frequency(float freq) { _frequency = freq; }

    void debug(bool debug) { doDebug = debug; }

    virtual void update(void) override;
};


#endif

