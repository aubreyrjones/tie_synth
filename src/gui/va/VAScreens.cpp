#include "../screen.hpp"
#include <audio/va/VASynth.hpp>
#include <array>

namespace gui {

constexpr std::array WaveformChoices {
    std::tuple{"Sine", WAVEFORM_SINE},
    std::tuple{"Tri", WAVEFORM_TRIANGLE},
    std::tuple{"Saw", WAVEFORM_BANDLIMIT_SAWTOOTH},
    std::tuple{"R.Saw", WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE},
    std::tuple{"Square", WAVEFORM_BANDLIMIT_SQUARE}
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
        // using namespace display;
        // using namespace colors;

        // if (!dirty) return;

        // main_oled.fillScreen(0); // clear the screen

        // main_oled.setCursor(4, 0);
        // main_oled.setTextSize(2);
        // main_oled.setTextColor(hotpink);
        // main_oled.print("The VAlley");

        // drawWidgets(&oscTypes);

        // dirty = false;

        drawHelper("The VAlley", colors::hotpink, 4, &oscTypes);
    }
} 
mainScreen; // instance


struct ScreenConstructor {
    ScreenConstructor() {
        Screen* parent = rootScreen->zipTo(South);

        parent->link(&mainScreen, South);
    }
} _screenConstructor;

}