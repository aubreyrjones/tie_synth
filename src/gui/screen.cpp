#include "screen.hpp"
#include "../audio/audio_externs.h"


namespace gui {

void Screen::drawHelper(const char* title, uint16_t titleColor, int titlePadding, Widget* firstWidget) {
    using namespace display;
    using namespace colors;

    if (!dirty) return;

    main_oled.fillScreen(0); // clear the screen

    main_oled.setCursor(titlePadding, 0);
    main_oled.setTextSize(2);
    main_oled.setTextColor(titleColor);
    main_oled.print(title);

    drawWidgets(firstWidget);

    dirty = false;
}


void Screen::flowWidgets(em::ivec const& tl, Widget* firstWidget) {
    if (!firstWidget) return;

    auto start = tl.y;
    do {
        firstWidget->position({tl.x, start});
        start += firstWidget->height();
    } while ( (firstWidget = firstWidget->next) );
}

void Screen::drawWidgets(Widget* firstWidget) {
    if (!firstWidget) return;
    do {
        firstWidget->draw(focused(*firstWidget));
    } while ( (firstWidget = firstWidget->next) );
}

//==========================================================

class HomeScreen : public Screen {
    audio::Control<float> headphoneVolume {"Vol.HP", 0.5f, {0.01, 1}, [](float vol) { output_amp.gain(vol);} };
    
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
        flowWidgets({0, start}, &volumes);

        volumes.setIncrements([](float v){ return 0.01f; }, [](float v){ return 0.01f; });
    }

    void draw() {
        using namespace display;
        using namespace colors;

        drawHelper("Synthburg", cornflowerblue, 12, &volumes);

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