#include "synth_additive.h"
#include <algorithm>
#include "utility/dspinst.h"


/// @brief Fastest available sinf() function.
constexpr auto fast_sin = arm_sin_f32;

/// @brief Fastest available cosf() function.
constexpr auto fast_cos = arm_cos_f32;

namespace resample {

/// @brief A pointer to a signal buffer and its length.
struct buffer {
    float *t;
    int len;
};

/// @brief Sample function definition. Takes an integer offset between -int_max and int_max.
using sample_func = float (*)(int, buffer);

/// @brief Sample from a buffer which is defined as zero everywhere except i = [0, wave.len - 1].
float sample_oneshot(int i, buffer wave) {
    if (i < 0 || i >= wave.len) {
        return 0;
    }

    return wave.t[i];
}

/// @brief Sample from a buffer which is treated as an infinite loop.
float sample_loop(int k, buffer wave) {
    if (k < 0) {
        int i = 5;
        while (k < 0) {
            k += wave.len;
            if (i-- < 0) {
                Serial.print("whuuhh?");
            }
        }
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

/// @brief Calculate the window (slowly) using fast_cos(). 
constexpr float window(float m, int M) {
    // see fast_window for a faster method. :)
    if (m >= 0 && m <= M) {
        return 0.42f - (0.5f * fast_cos((2 * PI * m) / M)) + (0.08f * fast_cos((4 * PI * m) / M));
    }

    return 0;
}

/// @brief 2D interpolation table for the window function.
constexpr std::array<float, 288> windowTableData {
	-1.3877787807814457e-17, 0.0664466094067262, 0.3399999999999999, 0.7735533905932737, 0.9999999999999999, 0.7735533905932739, 0.3400000000000001, 0.06644660940672628, -1.3877787807814457e-17,
	5.4227148311658535e-05, 0.07130431368527307, 0.3523669777650423, 0.7860489554219268, 0.9997530458445159, 0.7608448584223401, 0.32782574924213015, 0.06180187247046004, 0,
	0.00021705003118953348, 0.07637915135012596, 0.3649190590299332, 0.7983169339038443, 0.999012506236362, 0.7479381061971446, 0.3158513847025154, 0.05736580854888533, 0,
	0.0004888924578373699, 0.08167514056668268, 0.3776481610026512, 0.8103428612096713, 0.9977793491365277, 0.7348479835204595, 0.30408359740298396, 0.05313401470318674, 0,
	0.000870459096159959, 0.08719613215688699, 0.3905457477325219, 0.8221124524426587, 0.9960551857683569, 0.7215894163205329, 0.2925286074029615, 0.049101999079921826, 0,
	0.0013627329562085205, 0.09294579031742546, 0.4036028373440445, 0.8336116282055642, 0.9938422675549186, 0.7081773808980526, 0.2811921621448287, 0.045265200578958054, 0,
	0.001966971876186191, 0.09892757357342627, 0.41681001036910403, 0.8448265399206794, 0.9911434818409672, 0.6946268780658595, 0.27007953591374234, 0.041619008440034466, 0,
	0.0026847040201710415, 0.10514471601969971, 0.43015741916550887, 0.8557435948471693, 0.9879623464091124, 0.680952907437545, 0.25919553040520765, 0.038158781695585745, 0,
	0.003517722399287715, 0.11160020890099166, 0.4436347984071612, 0.8663494807404798, 0.9843030028025181, 0.6671704419205938, 0.2485444763910329, 0.03487986843793455, 0,
	0.004468078430611172, 0.1182967825820288, 0.45723147662855934, 0.876631190099276, 0.9801702084691397, 0.6532944024691261, 0.23813023647168965, 0.03177762484956904, 0,
	0.005538074550596406, 0.12523688895730933, 0.47093638880376354, 0.8865760439462158, 0.9755693277451404, 0.6393396331505311, 0.22795620890049972, 0.028847433945943773, 0,
	0.006730255902301835, 0.1324226843496502, 0.4847380899374274, 0.8961717150898134, 0.9705063216977416, 0.6253208765794344, 0.2180253324625291, 0.026084723981102016, 0,
	0.008047401118099193, 0.13985601294543293, 0.49862476964302743, 0.9054062508157456, 0.9649877368503081, 0.6112527497714307, 0.20834009238856532, 0.023484986467390702, 0,
	0.009492512221933244, 0.147538390813302, 0.5125842676809941, 0.9142680949571522, 0.95902069281497, 0.5971497204679089, 0.1989025272821028, 0.02104379376163705, 0,
	0.011068803677508544, 0.15547099055176725, 0.526604090427091, 0.9227461092948133, 0.9526128688605293, 0.5830260839820496, 0.18971423703487114, 0.018756816171370004, 0,
	0.012779690611027239, 0.1636546266097436, 0.5406714282390973, 0.9308295942395268, 0.9457724894457662, 0.5688959406147337, 0.1807763917041094, 0.016619838535996155, 0,
	0.014628776239280425, 0.17208974132253127, 0.554773173687621, 0.9385083087505672, 0.9385083087505672, 0.5547731736876215, 0.17208974132253121, 0.014628776239280453, 0,
	0.016619838535996106, 0.18077639170410914, 0.5688959406147334, 0.9457724894457661, 0.9308295942395269, 0.5406714282390972, 0.16365462660974353, 0.012779690611027246, 0,
	0.018756816171369955, 0.18971423703487098, 0.5830260839820492, 0.9526128688605292, 0.9227461092948134, 0.526604090427091, 0.15547099055176727, 0.011068803677508544, 0,
	0.021043793761637, 0.19890252728210264, 0.5971497204679086, 0.95902069281497, 0.9142680949571523, 0.5125842676809942, 0.147538390813302, 0.00949251222193323, 0,
	0.023484986467390646, 0.20834009238856516, 0.6112527497714307, 0.964987736850308, 0.9054062508157457, 0.4986247696430276, 0.13985601294543298, 0.008047401118099179, 0,
	0.02608472398110199, 0.21802533246252903, 0.6253208765794342, 0.9705063216977416, 0.8961717150898135, 0.4847380899374275, 0.1324226843496503, 0.006730255902301821, 0,
	0.028847433945943753, 0.2279562089004996, 0.639339633150531, 0.9755693277451404, 0.8865760439462159, 0.47093638880376365, 0.12523688895730936, 0.005538074550596378, 0,
	0.031777624849569, 0.2381302364716896, 0.653294402469126, 0.9801702084691396, 0.8766311900992763, 0.45723147662855956, 0.1182967825820289, 0.0044680784306112, 0,
	0.03487986843793456, 0.2485444763910329, 0.6671704419205938, 0.984303002802518, 0.8663494807404799, 0.44363479840716147, 0.11160020890099182, 0.0035177223992877427, 0,
	0.03815878169558573, 0.25919553040520765, 0.6809529074375451, 0.9879623464091123, 0.8557435948471696, 0.43015741916550915, 0.10514471601969985, 0.0026847040201710554, 0,
	0.0416190084400345, 0.27007953591374234, 0.6946268780658597, 0.9911434818409672, 0.8448265399206795, 0.4168100103691044, 0.09892757357342645, 0.0019669718761862187, 0,
	0.045265200578957936, 0.2811921621448283, 0.7081773808980522, 0.9938422675549184, 0.8336116282055642, 0.40360283734404445, 0.09294579031742539, 0.0013627329562085344, 0,
	0.04910199907992176, 0.29252860740296116, 0.7215894163205324, 0.9960551857683568, 0.8221124524426588, 0.39054574773252176, 0.08719613215688694, 0.000870459096159959, 0,
	0.05313401470318664, 0.30408359740298374, 0.7348479835204592, 0.9977793491365277, 0.8103428612096714, 0.3776481610026512, 0.08167514056668268, 0.0004888924578373699, 0,
	0.057365808548885296, 0.31585138470251517, 0.7479381061971443, 0.999012506236362, 0.7983169339038444, 0.3649190590299332, 0.07637915135012596, 0.00021705003118953348, 0,
	0.061801872470459936, 0.32782574924213004, 0.7608448584223401, 0.9997530458445159, 0.786048955421927, 0.3523669777650424, 0.07130431368527311, 5.4227148311658535e-05, 0,
};

/// @brief Approximate the window value using a lookup table. 
inline float fast_window(float windowOffset, int ki) {
    arm_bilinear_interp_instance_f32 windowTableInterpolator { 32, 9, windowTableData.data() };

    return arm_bilinear_interp_f32(&windowTableInterpolator, ki, windowOffset * (windowTableInterpolator.numRows - 1));
}

/// @brief The normalized sinc function. Runtime function with fast_sin.
float sinc(float x) {
    if (abs(x) <= std::numeric_limits<float>::epsilon()) return 1;
    const float nX = x * PI;
    return fast_sin(nX) / nX;
}


/// @brief Definition of modulo for positive floating point numbers.
/// @return (x % m) + fractional_part(x)
inline float mod(float x, int m) {
    while (x > m) {
        x -= m;
    }
    return x;
}

/// @brief Get just the fractional part of a floating point number.
inline float fract_part(float x) {
    float intPart;
    return modff(x, &intPart);
}


/// @brief Resample `input` into `output` with the given sample rates.
/// @param input the signal to be resampled
/// @param output the buffer to place the resampled signal
/// @param inputSampleRate the original sample rate of the input buffer
/// @param outputSampleRate the desired sample rate in the output buffer
/// @param samplePolicy one of the sample_func above, or something similar.
/// @param phase phase offset (in the input signal) to start playing.
/// @return the ending phase within the input buffer, pass back to this function to continue seamless playback within `input` on subsequent calls.
inline float windowed_sinc_interpolation(buffer input, buffer output, float inputSampleRate, float outputSampleRate, sample_func samplePolicy, float phase, bool profile) {
    const int windowSize = 8;
    const int halfWindow = windowSize / 2;

    const float sincScale = min(inputSampleRate, outputSampleRate) / inputSampleRate;
    const float sampleRatio = inputSampleRate / outputSampleRate;

    for (int j = 0; j < output.len; j++) {
        float J = j * sampleRatio + phase;
        int kLow = (int) ceilf(J - halfWindow);

        float accum = 0;
        for (int ki = 0; ki <= windowSize; ki++) {
            float sampleOffset = kLow + ki - J;
            float fractionalOffset = fract_part(sampleOffset + halfWindow);
            auto winScale = fast_window(fractionalOffset, ki);

            // After profiling, this sinc() call is the most costly part of this whole thing.
            // but it doesn't matter if I put it in a table, because profiling shows the new table isn't any faster
            // than the fast_sin() table already.
            float sincVal = sinc(sincScale * sampleOffset);
            accum += sincVal * winScale * samplePolicy(kLow + ki, input);
        }
        output.t[j] = min(1.f, outputSampleRate / inputSampleRate) * accum;
    }

    return mod(mod(output.len * sampleRatio, input.len) + phase, input.len);
}

/// @brief Play a pitch-shifted loop of the given buffer.
/// @param loop the buffer to loop
/// @param stream the output buffer
/// @param nativeSampleRate the sample rate of the input and output buffers
/// @param originalPitch the original perceived pitch of the loop, in Hz
/// @param targetPitch the desired pitch of the loop, in Hz
/// @param phase playback phase (see windowed_sinc_interpolation)
/// @return playback phase
inline float pitch_shift_looped(buffer loop, buffer stream, float nativeSampleRate, float originalPitch, float targetPitch, float phase, bool profile) {
    float shiftedRate = nativeSampleRate * (originalPitch / targetPitch);
    return windowed_sinc_interpolation(loop, stream, nativeSampleRate, shiftedRate, sample_loop, phase, profile);
}

/// @brief Play a single-cycle waveform looped at the given target pitch.
/// @param loop the single-cycle waveform to play
/// @param stream the output buffer
/// @param nativeSampleRate the sample rate of the output buffer
/// @param targetPitch the desired pitch of the loop in Hz
/// @param phase playback phase (see windowed_sinc_interpolation)
/// @return playback phase
inline float pitch_shift_single_cycle(buffer loop, buffer stream, float nativeSampleRate, float targetPitch, float phase, bool profile) {
    float originalPitch = nativeSampleRate / loop.len;
    return pitch_shift_looped(loop, stream, nativeSampleRate, originalPitch, targetPitch, phase, profile);
}

} //namespace resample


void AudioSynthAdditive::clearPartials() {
    std::fill(partialTable.begin(), partialTable.end(), 0.f);
}


void AudioSynthAdditive::update() {

    auto block = allocate();
    if (!block) return;

    // super naive additive algorithm follows for now, lol

    decltype(partialTable) workingArray;
    memcpy(workingArray.data(), partialTable.data(), sizeof(float) * partialTable.size());

    // calculate the waveform for this frame
    arm_rfft_fast_f32(&fftInstance, workingArray.data(), signal.data(), 1);

    int si = rawPlaybackPhase;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        int s =  32000 * signal.data()[si];
        block->data[i] = s;
        si = (si + 1) % signal_table_size;
    }
    rawPlaybackPhase = si;

    transmit(block);
    release(block);
}

// =====================================================================

// High accuracy 11th order Taylor Series Approximation
// input is 0 to 0xFFFFFFFF, representing 0 to 360 degree phase
// output is 32 bit signed integer, top 25 bits should be very good
//  https://www.pjrc.com/high-precision-sine-wave-synthesis-using-taylor-series/
static int32_t taylor(uint32_t ph)
{
	int32_t angle, sum, p1, p2, p3, p5, p7, p9, p11;

	if (ph >= 0xC0000000 || ph < 0x40000000) {                            // ph:  0.32
		angle = (int32_t)ph; // valid from -90 to +90 degrees
	} else {
		angle = (int32_t)(0x80000000u - ph);                        // angle: 2.30
	}
	p1 =  multiply_32x32_rshift32_rounded(angle, 1686629713) << 2;        // p1:  2.30
	p2 =  multiply_32x32_rshift32_rounded(p1, p1) << 1;                   // p2:  3.29
	p3 =  multiply_32x32_rshift32_rounded(p2, p1) << 2;                   // p3:  3.29
	sum = multiply_subtract_32x32_rshift32_rounded(p1, p3, 1431655765);   // sum: 2.30
	p5 =  multiply_32x32_rshift32_rounded(p3, p2);                        // p5:  6.26
	sum = multiply_accumulate_32x32_rshift32_rounded(sum, p5, 572662306);
	p7 =  multiply_32x32_rshift32_rounded(p5, p2);                        // p7:  9.23
	sum = multiply_subtract_32x32_rshift32_rounded(sum, p7, 109078534);
	p9 =  multiply_32x32_rshift32_rounded(p7, p2);                        // p9: 12.20
	sum = multiply_accumulate_32x32_rshift32_rounded(sum, p9, 12119837);
	p11 = multiply_32x32_rshift32_rounded(p9, p2);                       // p11: 15.17
	sum = multiply_subtract_32x32_rshift32_rounded(sum, p11, 881443);
	return sum <<= 1;                                                 // return:  1.31
}
// alternate forms which might be more efficient?
// https://twitter.com/josyboelen/status/1148227258693431296


float taylorf(uint32_t ph) {
    return taylor(ph) / 2147483648.f;
}


void AudioSynthOscBank::frequency(int bank, float f) {
    banks[bank].frequency(f);
}

void AudioSynthOscBank::Bank::frequency(float f) {
    constexpr float systemPhaseConstant = (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);

    if (f > systemNyquistFrequency) {
        f = systemNyquistFrequency;
    }

    fundamental = f;
    
    cutoff = -1;

    for (int i = 0; i < bankSize; i++) {
        float harmonicFreq = f * (i + 1);
        phaseIncrements[i] = harmonicFreq * systemPhaseConstant;

        if (cutoff < 0 && harmonicFreq > systemNyquistFrequency) {
            cutoff = i;
        }
    }

    if (cutoff < 0) {
        cutoff = bankSize;
    }
}

void AudioSynthOscBank::Bank::update() {
    for (int i = 0; i < bankSize; i++) {
        accumulators[i] += phaseIncrements[i];
    }
}

float AudioSynthOscBank::sample(Bank &b) {
    float accum = 0;

    for (int i = 0; i < b.cutoff; i++) {
        accum += voice.amplitudes[i] * taylorf(b.accumulators[i] + voice.phaseOffsets[i]);
    }

    return accum;
}

void AudioSynthOscBank::update() {
    auto block = allocate();
    if (!block) return;

    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        float s = 0;
        for (int j = 0; j < nBanks; j++) {
            banks[j].update();
            if (banks[j].active)
                s += sample(banks[j]);
        }

        block->data[i] = s * 32000;
    }

    transmit(block);
    release(block);
}

void AudioSynthOscBank::previewVoice(float *out, int nSamples) {
    Bank tempBank;
    for (int i = 0; i < bankSize; i++) {
        tempBank.phaseIncrements[i] = (i + 1) * (4294967296.0f / (nSamples));
    }
    tempBank.cutoff = bankSize;

    for (int i = 0; i < nSamples; i++) {
        tempBank.update();       
        out[i] = sample(tempBank);
    }
}

// ================================================================

/// @brief If this is throwing up problems, you've tried to initialize an FFT with an unsupported size.
template<bool flag = false> void static_no_match() { static_assert(flag, "FFT size (`N`) not supported."); }

template <int N>
void init_fft(arm_rfft_fast_instance_f32 &fftInstance) {
    if constexpr (N == 32) {
        arm_rfft_fast_init_32_f32(&fftInstance);
    }
    else if constexpr (N == 64) {
        arm_rfft_fast_init_64_f32(&fftInstance);
    }
    else if constexpr (N == 128) {
        arm_rfft_fast_init_128_f32(&fftInstance);
    }
    else if constexpr (N == 256) {
        arm_rfft_fast_init_256_f32(&fftInstance);
    }
    else if constexpr (N == 512) {
        arm_rfft_fast_init_512_f32(&fftInstance);
    }
    else if constexpr (N == 1024) {
        arm_rfft_fast_init_1024_f32(&fftInstance);
    }
    else if constexpr (N == 2048) {
        arm_rfft_fast_init_2048_f32(&fftInstance);
    }
    else if constexpr (N == 4096) {
        arm_rfft_fast_init_4096_f32(&fftInstance);
    }
    else {
        static_no_match();
    }
}

// AudioSynthIFFTBank::AudioSynthIFFTBank() : AudioStream(0, nullptr) {
//     init_fft<fftBufferLength>(fftInstance);
// }

