#ifndef synth_additive_h_
#define synth_additive_h_


#include <Arduino.h>     // github.com/PaulStoffregen/cores/blob/master/teensy4/Arduino.h
#include <AudioStream.h> // github.com/PaulStoffregen/cores/blob/master/teensy4/AudioStream.h
#include <arm_math.h>    
#include <array>

#include "timer-wheel.h"

/// @brief What's the Nyquist frequency for the environment?
static constexpr auto systemNyquistFrequency = AUDIO_SAMPLE_RATE_EXACT / 2.f;

class AudioSynthAdditive;


class Grain {

};


struct GrainEvent : public TimerEventInterface {
    AudioSynthAdditive *synth;

    GrainEvent(AudioSynthAdditive *s) : TimerEventInterface(), synth(s) {}

    virtual void execute() override;
};

class AudioSynthAdditive : public AudioStream {
public:
private:
    /// @brief If this is throwing up problems, you've set the `partial_table_size` to an unsupported value.
    template<bool flag = false> void static_no_match() { static_assert(flag, "FFT size not supported."); }

    TimerWheel grainScheduler {};

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

    int grainsOut = 0;

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

    void scheduleGrain();
    void reapGrain() { if (--grainsOut < 0) grainsOut = 0; }

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

// A struct to store a control point as a pair of time and a vector of amplitudes
template <int N> // a template parameter for the size of the amplitude array
struct ControlPoint {
  int t; // the integer sample time index of the control point
  float a[N]; // the vector of N amplitudes of the control point

  ControlPoint() {
    t = 0;
    std::fill(a, a + N, 0);
  }

  float& operator[](size_t i) { return a[i]; }
};

// A class to perform high-frequency interpolation between control points
template <int N, int M> // a template parameter for the size of the amplitude array, and the number of points in the sequence
class HighFrequencyInterpolator {
  private:
    float delta_a[4][N]; // a static array of delta-a values for each span and each amplitude
    int offset[4]; // a static array of sample step offsets for each span
    int span; // the current span index
    int step; // the current step index
  public:
    // A constructor that takes a buffer of control points and a sample rate and computes the delta-a and offset arrays
    void load(ControlPoint<N>* buffer, int sample_rate) {
      // loop through the control points and compute the delta-a and offset values
      for (int i = 0; i < (M - 1); i++) {
        // loop through the N amplitudes and compute the delta-a for each one
        for (int j = 0; j < N; j++) {
          // delta-a is the change in amplitude divided by the number of steps in the span
          delta_a[i][j] = (buffer[i+1].a[j] - buffer[i].a[j]) / (buffer[i+1].t - buffer[i].t);
        }
        // offset is the difference between the sample time index and the actual time index of the control point
        offset[i] = buffer[i].t - i * sample_rate;
      }
      // initialize the span and step values
      span = 0;
      step = 0;
    }

    // A method that returns the next delta-a vector
    float* next() {
      // if the span index is out of bounds, return a zero vector
      if (span >= 4) {
        static float zero[N] = {0}; // a static zero vector
        return zero;
      }
      // if the step index is equal to the offset, increment the span and reset the step
      if (step == offset[span]) {
        span++;
        step = 0;
      }
      // increment the step and return the delta-a vector
      step++;
      return delta_a[span];
    }
};


// A class to perform linear interpolation between control points
template <int N, int M> // the length of the amplitude array
class SequenceInterpolator {
  private:
    std::array<ControlPoint<N>, M> points {}; // the list of control points
    int index = 0; // the current index of the point
    int t = 0; // the current time
    float a[N]; // the current amplitude array

  public:
    // Constructor: takes a list of control points
    SequenceInterpolator() {
        for (int i = 0; i < M; i++) {
            points[i].t = (200 * 220);
            points[i].a[0] = 0.5f;
        }
    }

    // Returns the next interpolated amplitude array
    float* next() {
      // Get the current and next control points
      ControlPoint<N> & p1 = points[index];
      ControlPoint<N> & p2 = points[(index + 1) % points.size()]; // use modulo to loop back to the first point

      // If the current time is equal to the next point's time, increment the index and return the next point's amplitude array
      if (t == p2.t) {
        t = 0;
        index = (index + 1) % points.size(); // use modulo to loop back to the first point
        for (int i = 0; i < N; i++) {
          a[i] = p2.a[i];
        }
        return a;
      }

      // Otherwise, perform linear interpolation between the two points for each element of the amplitude array
      for (int i = 0; i < N; i++) {
        a[i] = p1.a[i] + (p2.a[i] - p1.a[i]) * (t) / p2.t;
      }

      // Increment the current time by 1
      t++;

      // Return the interpolated amplitude array
      return a;
    }

    ControlPoint<N>& operator[](size_t i) { return points[i]; }
};


class AudioSynthOscBank : public AudioStream {
public:
    static constexpr auto nBanks = 4;
    static constexpr auto bankSize = 16;
    static constexpr auto nControlPoints = 5;

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

    SequenceInterpolator<bankSize * 2, nControlPoints> voiceInterpolator {};

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

    decltype(voiceInterpolator)& getVoice() { return voiceInterpolator; }

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

