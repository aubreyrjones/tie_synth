#include "screen.hpp"


namespace gui {


//==========================================================

class HomeScreen : public Screen {
    audio::Control<float> headphoneVolume {0.5f, {0.01, 1}, [](float vol) { } };
    audio::Control<float> lineoutVolume {0.5f, {0.01, 1}, [](float vol) { } };

    audio::Control<byte> testControl1 {0, {0, 255}, [](byte) { } };
    audio::Control<byte> testControl2 {0, {0, 127}, [](byte) { } };

    DualNumericalWidget<float> volumes;
    DualNumericalWidget<byte> something;

public:
    HomeScreen() : 
        volumes("Vol.HP", "Vol.Ln", headphoneVolume, lineoutVolume), 
        something("Hi", "Katie", testControl1, testControl2){
        volumes.link(something);

        focusedWidget = &volumes;

        // layout
        auto start = 18;
        volumes.position({0, start});
        something.position({0, start += volumes.height()});

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