#include "../screen.hpp"
#include <audio/va/VASynth.hpp>

namespace gui {

struct VAScreen : public Screen {

    DualNumericalWidget<float> testAmpFreq;

    VAScreen() : Screen(), testAmpFreq("Amp.", "Freq.", audio::va_module.amplitude, audio::va_module.frequency) {
        focusedWidget = &testAmpFreq;

        auto start = 18;
        testAmpFreq.position({0, start});

        testAmpFreq.setIncrements(
            [](float v){ return 0.01f; },
            DualNumericalWidget<float>::_default_incr
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

        testAmpFreq.draw(focused(testAmpFreq));

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