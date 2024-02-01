#ifndef synth_additive_h_
#define synth_additive_h_


#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    // github.com/PaulStoffregen/cores/blob/master/teensy4/arm_math.h
#include <array>


struct NCO {
    uint32_t phase_accumulator = 0;
	uint32_t phase_increment = 0;
	uint32_t phase_offset = 0;
    float frequency = 440;
    float outputSampleRate;
    float halfRate, sampleRatePhaseIncrementConstant;
    
    float* table;

    inline NCO(float outputRate, float* table) : table(table) {
        setOutputRate(outputRate);
    }

    inline void setOutputRate(float rate) {
        outputSampleRate = rate;
        halfRate = rate / 2.f;
        sampleRatePhaseIncrementConstant = 4294967296.0f / rate;
        
        setFrequency(frequency); // reset our frequency control words
    }

    inline void setFrequency(float freq) {
        frequency = freq;
        if (freq < 0.0f) {
			freq = 0.0;
		} else if (freq > halfRate) { // no frequency above nyquist
			freq = halfRate;
		}
		phase_increment = freq * (4294967296.0f / outputSampleRate);
		if (phase_increment > 0x7FFE0000u) phase_increment = 0x7FFE0000;
    }

    inline void step() {
        phase_accumulator += phase_increment;
    }

    inline float sample() {
        auto index = phase_accumulator >> 24; // index into 256-entry table.
        auto residue = (phase_accumulator & 0x0fff) / (float) 0x0fff; // interpolation factor
        
        auto nextIndex = index + 1;
        if (nextIndex > 255) {
            nextIndex = 0;
        }

        auto s = table[index];
        auto s1 = table[nextIndex];

        return s + residue * (s1 - s);
    }
};


class AudioSynthAdditive : public AudioStream {
protected:
    static constexpr auto partial_table_size = 256;
    static constexpr auto signal_table_size = partial_table_size;
    std::array<float, partial_table_size> partialTable; // frequency domain, packed amplitude and phase
    std::array<float, signal_table_size> signal; // time domain, amplitude

    arm_rfft_fast_instance_f32 fftInstance;

    float _frequency = 440;

    float playbackPhase = 0;
    bool useWindow = false;

public:
    AudioSynthAdditive(void) : AudioStream(0, NULL) {
        arm_rfft_fast_init_256_f32(&fftInstance);

        partialTable[2] = 40;
    }

    /// @brief Get a reference to the partial array.
    /// @return A mutable reference to the partial array.
    std::array<float, partial_table_size>& partials() { return partialTable; }

    std::array<float, signal_table_size>& samples() { return signal; }

    void frequency(float freq) { _frequency = freq; }

    void window(bool doWindowing) { useWindow = doWindowing; }

    virtual void update(void) override;
};


#endif

