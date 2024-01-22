#include "screen.hpp"
#include "colors.hpp"


namespace gui {


void DualNumericalWidget::draw(em::ivec const& topLeft, bool focused) {
    using namespace display;
    using namespace colors;

    auto bg = focused ? colors::white : colors::black;
    auto fg = focused ? colors::black : colors::white;

    if (focused) {
        main_oled.fillRect(topLeft.x, topLeft.y, 126, 12, colors::white);
    }
    else {
        main_oled.drawRect(topLeft.x, topLeft.y, 126, 12, colors::white);
    }

    main_oled.setCursor(topLeft.x + 2, topLeft.y + 2);
    main_oled.setTextSize(1);
    main_oled.setTextColor(fg, bg);

    main_oled.print(aLabel);
    main_oled.print(": ");
    main_oled.print(aVal, DEC);
    main_oled.print(" ");
    main_oled.print(bLabel);
    main_oled.print(": ");
    main_oled.print(bVal, DEC);
}


//==========================================================

class HomeScreen : public Screen {
    DualNumericalWidget volumes;
    DualNumericalWidget something;
public:
    HomeScreen() : volumes("Phones", "LnOut", 64, 64), something("Hi!!", "Katie!!", 0, 0) {
        volumes.link(something);

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

        auto start = 18;

        volumes.draw({2, start += volumes.height()}, focused(volumes));
        something.draw({2, start += something.height()}, focused(something));

        dirty = false;
    }
};

//===========================================================

HomeScreen home;


Screen* rootScreen = &home;
Screen* activeScreen = &home;


}