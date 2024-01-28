#include "../screen.hpp"
#include <audio/va/VASynth.hpp>
#include <array>

namespace gui {

constexpr std::array WaveformChoices {
    std::tuple{"Sine", WAVEFORM_SINE},
    std::tuple{"Tri", WAVEFORM_TRIANGLE},
    std::tuple{"Saw", WAVEFORM_BANDLIMIT_SAWTOOTH},
    std::tuple{"R.Saw", WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE},
    std::tuple{"Square", WAVEFORM_BANDLIMIT_SQUARE},
    std::tuple{"Pulse", WAVEFORM_BANDLIMIT_PULSE}
};

constexpr std::array FilterChoices {
    std::tuple{"LowP", 0},
    std::tuple{"BandP", 1},
    std::tuple{"HighP", 2}
};

static struct VAScreen : public Screen {

    DualNumericalWidget<float> mixOsc12;
    DualNumericalWidget<float> mixOsc34;
    DualNumericalWidget<float> filterSettings;
    DualWidget<ChoiceWidget, NumericalWidget<float>> filterType;

    VAScreen() : 
        Screen(),
        mixOsc12(audio::va_module.mix.osc1, audio::va_module.mix.osc2),
        mixOsc34(audio::va_module.mix.osc3, audio::va_module.wavefolderAmount),
        filterSettings(audio::va_module.filterCutoffFreq, audio::va_module.filterResonance),
        filterType(ChoiceWidget(audio::va_module.filterSwitch, meta::length_erase_array(FilterChoices)), NumericalWidget(audio::va_module.frequency))
        
    {
        mixOsc12.link(mixOsc34).link(filterType).link(filterSettings);

        focusedWidget = &mixOsc12;

        flowWidgets({0, 18}, &mixOsc12);

        mixOsc12.setIncrements(audio::small_f, audio::small_f);
        mixOsc34.setIncrements(audio::small_f, audio::small_f);

        filterSettings.setIncrements(
            [](float v){ return 10.f; },
            [](float v){ return 0.01f; }
        );

    }

    void draw() override {
        using namespace display;

        drawHelper("The VAlley", colors::hotpink, 4, &mixOsc12);
    }
} 
mainScreen; // instance

static struct Osc12Screen : public Screen {
    DualWidget<ChoiceWidget, ChoiceWidget> oscTypes;
    DualNumericalWidget<float> phase;
    DualNumericalWidget<float> pulseWidth;
    DualNumericalWidget<int> detune;
    
    Osc12Screen() : 
        Screen(), 
        oscTypes(ChoiceWidget(audio::va_module.osc1.waveType, meta::length_erase_array(WaveformChoices)), ChoiceWidget(audio::va_module.osc2.waveType, meta::length_erase_array(WaveformChoices))),
        phase(audio::va_module.osc1.phase, audio::va_module.osc2.phase),
        pulseWidth(audio::va_module.osc1.pulseWidth, audio::va_module.osc2.pulseWidth),
        detune(audio::va_module.osc1.detune, audio::va_module.osc2.detune)

    {
        oscTypes.link(phase).link(pulseWidth);

        focusedWidget = &oscTypes;

        // phase.setIncrements(

        // );

        pulseWidth.setIncrements(
            [](float v){ return 0.01f; },
            [](float v){ return 0.01f; }
        );

        flowWidgets({0, 18}, &oscTypes);
    }

    void draw() override {
        using namespace display;

        drawHelper("Osc1 Osc2", colors::hotpink, 4, &oscTypes);
    }
} 
osc1Screen; // instance

static struct Osc3Screen : public Screen {
    DualWidget<ChoiceWidget, NumericalWidget<int>> oscTypes;
    DualNumericalWidget<float> mixRow0;
    DualNumericalWidget<float> mixRow1;
    
    Osc3Screen() : 
        Screen(), 
        oscTypes(ChoiceWidget(audio::va_module.osc3.waveType, meta::length_erase_array(WaveformChoices)), NumericalWidget<int>(audio::va_module.osc3.detune)),
        mixRow0(audio::va_module.osc3.osc1Level, audio::va_module.osc3.osc2Level),
        mixRow1(audio::va_module.osc3.inLLevel, audio::va_module.osc3.inRLevel)

    {
        oscTypes.link(mixRow0).link(mixRow1);

        focusedWidget = &oscTypes;

        mixRow0.setIncrements(audio::small_f, audio::small_f);
        mixRow1.setIncrements(audio::small_f, audio::small_f);

        flowWidgets({0, 18}, &oscTypes);
    }

    void draw() override {
        using namespace display;

        drawHelper("Osc3", colors::hotpink, 4, &oscTypes);
    }
} 
osc3Screen; // instance


static struct ScreenConstructor {
    ScreenConstructor() {
        // link our screens together
        mainScreen.link(&osc1Screen, East);
        osc1Screen.link(&osc3Screen, East);

        // link to main graph

        Screen* parent = rootScreen->zipTo(South);
        parent->link(&mainScreen, South);
        
    }
} _screenConstructor;

}