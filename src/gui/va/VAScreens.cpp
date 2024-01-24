#include "../screen.hpp"
#include <audio/va/VASynth.hpp>

namespace gui {

struct VAScreen : public Screen {

    DualNumericalWidget<int> oscTypes;
    DualNumericalWidget<float> testAmpFreq;
    DualNumericalWidget<float> filterSettings;
    DualNumericalWidget<int> filterType;


    VAScreen() : 
        Screen(), 
        oscTypes(audio::va_module.osc1Type, audio::va_module.osc2Type), 
        testAmpFreq(audio::va_module.osc12Mix, audio::va_module.frequency),
        filterSettings(audio::va_module.filterCutoffFreq, audio::va_module.filterResonance),
        filterType(audio::va_module.filterSwitch, audio::va_module.dummyContrl)
        
    {
        oscTypes.link(testAmpFreq).link(filterType).link(filterSettings);

        focusedWidget = &oscTypes;

        auto start = 18;
        flowWidgets({0, 18}, &oscTypes);

        testAmpFreq.setIncrements(
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
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        main_oled.setCursor(4, 0);
        main_oled.setTextSize(2);
        main_oled.setTextColor(hotpink);
        main_oled.print("The VAlley");

        drawWidgets(&oscTypes);

        dirty = false;
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