#include "screen.hpp"


namespace gui {



//==========================================================

class HomeScreen : public Screen {
    DualNumericalWidget<byte> volumes;
    DualNumericalWidget<byte> something;

public:
    HomeScreen() : volumes("HP", "LnOut", 64, 64), something("Hi", "Katie", 0, 0) {
        volumes.link(something);

        auto start = 18;
        volumes.position({0, start += volumes.height()});
        something.position({0, start += something.height()});

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

        dirty = false;
    }
};

//===========================================================

HomeScreen home;


Screen* rootScreen = &home;
Screen* activeScreen = &home;


}