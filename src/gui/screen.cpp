#include "screen.hpp"


namespace gui {


//==========================================================

class HomeScreen : public Screen {
    audio::Control<float> headphoneVolume {"Vol.HP", 0.5f, {0.01, 1}, [](float vol) { } };
    audio::Control<float> lineoutVolume {"Vol.Ln", 0.5f, {0.01, 1}, [](float vol) { } };

    audio::Control<byte> testControl1 {"Hi", 0, {0, 255}, [](byte) { } };
    audio::Control<byte> testControl2 {"Katie", 0, {0, 127}, [](byte) { } };
    audio::Control<byte> testControl3 {"VA.Lvl", 0, {0, 127}, [](byte) { } };
    audio::Control<byte> testControl4 {"Grn.Lvl", 0, {0, 127}, [](byte) { } };
    audio::Control<byte> testControl5 {"In.Lvl", 0, {0, 127}, [](byte) { } };
    audio::Control<byte> testControl6 {"Mic.Lvl", 0, {0, 127}, [](byte) { } };



    DualNumericalWidget<float> volumes;
    DualNumericalWidget<byte> something;
    DualNumericalWidget<byte> something2;
    DualNumericalWidget<byte> something3;



public:
    HomeScreen() : 
        volumes(headphoneVolume, lineoutVolume), 
        something(testControl1, testControl2),
        something2(testControl3, testControl4),
        something3(testControl5, testControl6){

        volumes.link(something);
        something.link(something2);
        something2.link(something3);

        focusedWidget = &volumes;

        // layout
        auto start = 18;
        volumes.position({0, start});
        something.position({0, start += volumes.height()});
        something2.position({0, start += something.height()});
        something3.position({0, start += something2.height()});


        volumes.setIncrements([](float v){ return 0.01f; }, [](float v){ return 0.01f; });
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
        something2.draw(focused(something2));
        something3.draw(focused(something3));

        dirty = false;
    }
};

//===========================================================

HomeScreen home;


Screen* rootScreen = &home;
Screen* activeScreen = &home;


Screen* go_to_screen(Screen *s) {
    if (!s || s == activeScreen) return activeScreen;

    activeScreen->sully();
    activeScreen = s;

    return s;
}

}