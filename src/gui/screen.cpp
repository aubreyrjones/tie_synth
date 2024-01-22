#include "screen.hpp"
#include "colors.hpp"


namespace gui {


//==========================================================

class HomeScreen : public Screen {
public:
    void draw() {
        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        main_oled.setCursor(12, 16);
        main_oled.setTextSize(2);
        main_oled.setTextColor(cornflowerblue);
        main_oled.print("Synthburg");

        dirty = false;
    }
};

//===========================================================

HomeScreen home;


Screen* rootScreen = &home;
Screen* activeScreen = &home;


}