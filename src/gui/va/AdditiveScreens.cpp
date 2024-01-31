#include "../screen.hpp"
#include "../../audio/additive/AddSynth.hpp"

namespace gui {


static struct AdditiveScreen : public Screen {

    DualWidget<NumericalWidget<int>, NumericalWidget<float>> filterTypeFreq;

    AdditiveScreen() : 
        Screen(),
        filterTypeFreq(NumericalWidget(audio::as_module.debug), NumericalWidget(audio::as_module.frequency))
        
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

    virtual bool showPerf() override { return false; }

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
                auto& part = reinterpret_cast<std::tuple<float, float>&>(audio::as_module.partials()[partial * 2]);
                auto polar = em::to_polar(part);
                float r = std::get<0>(polar);
                float phase = std::get<1>(polar);

                if (partial == selectedPartial) 
                    main_oled.drawRect(i * 4, y, 4, 32, colors::cornflowerblue);

                int boxHeight = (r / 100) * 15;
                main_oled.fillRect(i * 4 + 1, yh - boxHeight, 2, boxHeight, colors::darkorange);

                boxHeight = (phase / PI) * 8;
                main_oled.fillRect(i * 4 + 1, yh + 8, 2, boxHeight, colors::hotpink);

                partial++;
            }
        }

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
        auto& part = reinterpret_cast<std::tuple<float, float>&>(audio::as_module.partials()[selectedPartial * 2]);

        auto polar = em::to_polar(part);

        float r = std::get<0>(polar);
        float phase = std::get<1>(polar);

        if (event == WidgetInput::RIGHT_DECR) {
            r = em::clamp_incr(0, r, 100.f, 0.05f + r * -0.2f);
        }
        else if (event == WidgetInput::RIGHT_INCR) {
            r = em::clamp_incr(0, r, 100.f, 0.05f + r * 0.2f);
        }
        else if (event == WidgetInput::RIGHT_REL) {
            r = 0;
            phase = 0;
        }

        constexpr float phaseIncr = PI / 32.f;

        if (event == WidgetInput::LEFT_INCR) {
            phase -= phaseIncr;
        }
        else if (event == WidgetInput::LEFT_DECR) {
            phase += phaseIncr;
        }
        else if (event == WidgetInput::LEFT_REL) {
            phase = 0;
        }

        part = em::to_cartesean({r, phase});

        sully();
    }

    virtual bool hasScope() { return true; }


    int offset = 0;
    int direction = 1;
    virtual void drawScope() {
        display::draw_buffer_in_scope(additive1.samples().data() + offset);
        if (offset > 0 && offset < 128) {
            offset += direction;
        }
        else if (offset <= 0) {
            direction = 1;
            offset++;
        }
        else if (offset >= 128) {
            direction = -1;
            offset--;
        }
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