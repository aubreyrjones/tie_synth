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

struct VAScreen : public Screen {

    DualWidget<ChoiceWidget, ChoiceWidget> oscTypes;
    DualNumericalWidget<float> mixAndFreq;
    DualNumericalWidget<float> filterSettings;
    DualWidget<ChoiceWidget, NumericalWidget<int>> filterType;

    VAScreen() : 
        Screen(), 
        oscTypes(ChoiceWidget(audio::va_module.osc1Type, meta::length_erase_array(WaveformChoices)), ChoiceWidget(audio::va_module.osc2Type, meta::length_erase_array(WaveformChoices))), 
        mixAndFreq(audio::va_module.osc12Mix, audio::va_module.frequency),
        filterSettings(audio::va_module.filterCutoffFreq, audio::va_module.filterResonance),
        filterType(ChoiceWidget(audio::va_module.filterSwitch, meta::length_erase_array(FilterChoices)), NumericalWidget(audio::va_module.dummyContrl))
        
    {
        oscTypes.link(mixAndFreq).link(filterType).link(filterSettings);

        focusedWidget = &oscTypes;

        flowWidgets({0, 18}, &oscTypes);

        mixAndFreq.setIncrements(
            [](float v){ return 0.01f; },
            [](float v){ return 1.f; }
        );

        filterSettings.setIncrements(
            [](float v){ return 10.f; },
            [](float v){ return 0.01f; }
        );

    }

    void draw() override {
        using namespace display;

        drawHelper("The VAlley", colors::hotpink, 4, &oscTypes);
    }
} 
mainScreen; // instance

struct Osc12Screen : public Screen {
    DualWidget<ChoiceWidget, ChoiceWidget> oscTypes;
    DualNumericalWidget<float> phase;
    DualNumericalWidget<float> pulseWidth;
    
    Osc12Screen() : 
        Screen(), 
        oscTypes(ChoiceWidget(audio::va_module.osc1.waveType, meta::length_erase_array(WaveformChoices)), ChoiceWidget(audio::va_module.osc2.waveType, meta::length_erase_array(WaveformChoices))),
        phase(audio::va_module.osc1.phase, audio::va_module.osc2.phase),
        pulseWidth(audio::va_module.osc1.pulseWidth, audio::va_module.osc2.pulseWidth)
    {
        oscTypes.link(phase).link(pulseWidth);

        focusedWidget = &oscTypes;

        flowWidgets({0, 18}, &oscTypes);
    }

    void draw() override {
        using namespace display;

        drawHelper("Osc1 Osc2", colors::hotpink, 4, &oscTypes);
    }
} 
osc1Screen; // instance


struct ScreenConstructor {
    ScreenConstructor() {
        // link our screens together
        mainScreen.link(&osc1Screen, East);

        // link to main graph

        Screen* parent = rootScreen->zipTo(South);
        parent->link(&mainScreen, South);
        
    }
} _screenConstructor;

}