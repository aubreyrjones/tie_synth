#include "../screen.hpp"
#include "../../audio/additive/AddSynth.hpp"

namespace gui {

static struct AdditiveScreen : public Screen {

    DualWidget<NumericalWidget<float>, NumericalWidget<float>> filterTypeFreq;

    AdditiveScreen() : 
        Screen(),
        filterTypeFreq(NumericalWidget(audio::as_module.frequency), NumericalWidget(audio::as_module.frequency))
        
    {
        focusedWidget = &filterTypeFreq;

        flowWidgets({0, 18}, &filterTypeFreq);
    }

    void draw() override {
        using namespace display;

        drawHelper("Addington", colors::darkorange, 10, &filterTypeFreq);
    }
} 
mainScreen; // instance


static struct PartialEditor : public Screen {
    int selectedPartial = 1;

    PartialEditor() : Screen()
    {}

    void draw() override {
        using namespace display;

        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        int partial = 0;

        for (int row = 0; row < 4; row++) {
            int y = row * 32;
            int yh = y + 16;

            main_oled.drawFastHLine(0, yh, 128, colors::darkgrey);

            for (int i = 0; i < 32; i++) {
                float s = audio::as_module.partials()[partial * 2];
                float sp = sqrtf(abs(s)) / 10.f;

                int boxHeight = sp * 15;
                if (partial == selectedPartial) 
                    main_oled.drawRect(i * 4, y, 4, 32, colors::cornflowerblue);

                if (s < 0) {
                    main_oled.fillRect(i * 4 + 1, yh, 2, boxHeight, colors::darkorange);
                }
                else {
                    main_oled.fillRect(i * 4 + 1, yh - boxHeight, 2, boxHeight, colors::darkorange);
                }

                partial++;
            }
        }

        main_oled.setCursor(128 - 64, 0);
        main_oled.setTextColor(colors::white, colors::black);
        
        float& s = audio::as_module.partials()[selectedPartial * 2];
        float sp = copysignf(sqrtf(abs(s)), s);

        main_oled.print(sp);

        dirty = false;
    }

    virtual void nextWidget() override {
        selectedPartial = (selectedPartial + 1) % 128;
        sully();
    }

    virtual void prevWidget() override { 
        selectedPartial = selectedPartial - 1;
        if (selectedPartial < 0) selectedPartial = 127;
        sully();
    }

    virtual void handleInput(WidgetInput const& event) {
        float& s = audio::as_module.partials()[selectedPartial * 2];
        float sp = abs(s) > 1 ? copysignf(sqrtf(abs(s)), s) : s;

        if (event == WidgetInput::RIGHT_DECR) {
            sp = em::clamp_incr(-10.f, sp, 10.f, -0.2f);
        }
        else if (event == WidgetInput::RIGHT_INCR) {
            sp = em::clamp_incr(-10.f, sp, 10.f, 0.2f);
        }
        else if (event == WidgetInput::RIGHT_REL) {
            sp = 0;   
        }

        s = abs(sp) > 1 ? copysignf(sp * sp, sp) : sp;

        constexpr float phaseIncr = 1.f; //PI / 8.f;

        float& p = audio::as_module.partials()[selectedPartial * 2 + 1];
        if (event == WidgetInput::LEFT_DECR) {
            p -= phaseIncr;
        }
        else if (event == WidgetInput::LEFT_INCR) {
            p += phaseIncr;
        }
        else if (event == WidgetInput::LEFT_REL) {
            p = 0;
        }

        sully();
    }


} partialEditor;


static struct ScreenConstructor {
    ScreenConstructor() {
        // link our screens together
        mainScreen.link(&partialEditor, East);

        // link to main graph

        Screen* parent = rootScreen->zipTo(North);
        parent->link(&mainScreen, North);
        
    }
} _screenConstructor;

}