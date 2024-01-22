#include "screen.hpp"


namespace gui {


//==========================================================

class HomeScreen : public Screen {
    DualNumericalWidget<byte> volumes;
    DualNumericalWidget<byte> something;
    DualNumericalWidget<float> another;
    DualNumericalWidget<float> one;
    DualNumericalWidget<float> last;

public:
    HomeScreen() : volumes("Vol.HP", "Vol.Ln", 64, 64), something("Hi", "Katie", 0, 0), another("I \x03\x03\x03\x03\x03", "You!", 14.27, 127.78), one("A whole", "Bunch!", 13, 37),
    last("Does this", "Fit?", 13, 37) {
        volumes.link(something);
        something.link(another);
        another.link(one);
        one.link(last);

        // layout
        auto start = 18;
        volumes.position({0, start});
        something.position({0, start += volumes.height()});
        another.position({0, start += something.height()});
        one.position({0, start += another.height()});
        last.position({0, start += one.height()});

        another.setLimits(1, 20000, 1, 4000);
        another.setIncrements(DualNumericalWidget<float>::_default_incr, [](float v){ return max(0.01f, logf(v) / 2.f); });

        focusedWidget = &volumes;
    }

    void draw() {
        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        main_oled.setCursor(12, 0);
        main_oled.setTextSize(2);
        main_oled.setTextColor(cornflowerblue);
        main_oled.print("Synthburg");

        volumes.draw(focused(volumes));
        something.draw(focused(something));
        another.draw(focused(another));
        one.draw(focused(one));
        last.draw(focused(last));

        dirty = false;
    }
};

//===========================================================

HomeScreen home;


Screen* rootScreen = &home;
Screen* activeScreen = &home;


}