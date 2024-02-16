#include "screen.hpp"
#include "../audio/audio_externs.h"


namespace gui {

constexpr std::array cc_table {
    Input::NAV_ROTATE,
    Input::RIGHT_ROTATE,
    Input::LEFT_ROTATE,
    Input::NAV_SOUTH,
    Input::NAV_EAST,
    Input::NAV_NORTH,
    Input::NAV_WEST,
    Input::NAV_CENTER,
    Input::RIGHT_PUSH,
    Input::LEFT_PUSH
};

constexpr int cc_table_size = cc_table.size();

bool handle_nav_arrows(int cc, bool push) {
    auto direction = -1;
    switch (cc) {
    case 3:
        direction = gui::South;
        break;
    case 4:
        direction = gui::East;
        break;
    case 5:
        direction = gui::North;
        break;
    case 6:
        direction = gui::West;
        break;
    }

    if (direction < 0)
        return false; // not our CC

    if (!push) { // move on release
        gui::go_to_screen(gui::activeScreen->nextScreen(direction));
    }

    return true;
}

InputEvent decode_event(int cc, int val) {
    if (cc < 0 || cc > cc_table_size) return InputEvent {Input::INVALID, InputTransition::DECR};
    
    Input in = cc_table[cc];
    InputTransition trans;

    if (cc == 0 || cc == 1 || cc == 2) {
        trans = val ? InputTransition::INCR : InputTransition::DECR;
    }
    else {
        trans = val ? InputTransition::PRESS : InputTransition::RELEASE;
    }

    return { in, trans };
}

void handleUserInput(int cc, int val) {
    InputEvent event = decode_event(cc, val);

    if (gui::activeScreen->handleInput(event)) {
        return;
    }
    else if (handle_nav_arrows(cc, val)) {
      //intentionally blank.
    }    
}


bool Screen::handleInput(InputEvent const& ev) {
    if (ev.in == Input::NAV_ROTATE) {
        if (ev.trans == InputTransition::DECR) {
            prevWidget();
        }
        else {
            nextWidget();
        }
        return true;
    }
    if (ev.in == Input::LEFT_PUSH || ev.in == Input::LEFT_ROTATE || ev.in == Input::RIGHT_PUSH || ev.in == Input::RIGHT_ROTATE) {
        passInputToWidget(ev);
        return true;
    }

    return false;
}

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

    audio::Control<float> mixVA {"Mix.VA", 0, {0, 1}, [](float g) { output_mixer.gain(0, g); } };
    audio::Control<float> mixAdditive {"Mix.Add", 1, {0, 1}, [](float g) { output_mixer.gain(1, g);} };
    
    audio::Control<byte> testControl5 {"In.Lvl", 0, {0, 127}, [](byte) { } };
    audio::Control<byte> testControl6 {"Mic.Lvl", 0, {0, 127}, [](byte) { } };

    DualNumericalWidget<float> volumes;
    DualNumericalWidget<byte> something;
    DualNumericalWidget<float> mixVAAdditive;
    DualNumericalWidget<byte> something3;

public:
    HomeScreen() : 
        volumes(headphoneVolume, lineoutVolume), 
        something(testControl1, testControl2),
        mixVAAdditive(mixVA, mixAdditive),
        something3(testControl5, testControl6){

        volumes.link(something);
        something.link(mixVAAdditive);
        mixVAAdditive.link(something3);

        focusedWidget = &volumes;

        // layout
        auto start = 18;
        flowWidgets({0, start}, &volumes);

        volumes.setIncrements(audio::small_f, audio::small_f);
        mixVAAdditive.setIncrements(audio::small_f, audio::small_f);
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