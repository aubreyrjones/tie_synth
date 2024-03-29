#include "../screen.hpp"
#include "../../audio/additive/AddSynth.hpp"

namespace gui {


static struct AdditiveScreen : public Screen {

    DualNumericalWidget<float> mixes;
    DualWidget<NumericalWidget<int>, NumericalWidget<float>> filterTypeFreq;

    AdditiveScreen() : 
        Screen(),
        mixes(audio::as_module.spectralMix, audio::as_module.banksMix),
        filterTypeFreq(NumericalWidget(audio::as_module.debug), NumericalWidget(audio::as_module.frequency))
        
    {
        mixes.link(filterTypeFreq);
        focusedWidget = &mixes;

        mixes.setIncrements(audio::small_f, audio::small_f);

        flowWidgets({0, 18}, &mixes);
    }

    void draw() override {
        using namespace display;

        drawHelper("Addington", colors::darkorange, 10, &mixes);
    }
} 
mainScreen; // instance


struct PartialEditor : public Screen {
    int firstPartialOffset = 0;
    int selectedPartial = 0;

    PartialEditor() : Screen() {}

    PartialEditor(int offset) : Screen(), firstPartialOffset(offset)
    {
    }

    virtual bool showPerf() override { return false; }

    int referenced() {
        return firstPartialOffset + selectedPartial;
    }

    void draw() override {
        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        main_oled.setCursor(0, 0);
        main_oled.print(firstPartialOffset / 128);

        int partial = 0;

        for (int row = 0; row < 4; row++) {
            int y = row * 32;
            int yh = y + 16;

            main_oled.drawFastHLine(0, yh, 128, colors::darkgrey);

            for (int i = 0; i < 32; i++) {
                auto& part = reinterpret_cast<std::tuple<float, float>&>(audio::as_module.partials()[(partial + firstPartialOffset) * 2]);
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

    virtual void passInputToWidget(InputEvent const& event) override {
        auto& part = reinterpret_cast<std::tuple<float, float>&>(audio::as_module.partials()[referenced() * 2]);

        auto polar = em::to_polar(part);

        float r = std::get<0>(polar);
        float phase = std::get<1>(polar);

        if (event.in == Input::RIGHT_ROTATE) {
            if (event.trans == InputTransition::DECR) {
                r = em::clamp_incr(0, r, 500.f, 0.05f + r * -0.2f);
            }
            else {
                r = em::clamp_incr(0, r, 500.f, 0.05f + r * 0.2f);
            }
        }
        else if (event.in == Input::RIGHT_PUSH && event.trans == InputTransition::RELEASE) {
            r = 0; 
            phase = 0;
        }

        constexpr float phaseIncr = PI / 32.f;

        if (event.in == Input::LEFT_ROTATE) {
            if (event.trans == InputTransition::DECR) {
                phase -= phaseIncr;
            }
            else {
                phase += phaseIncr;
            }
        }
        else if (event.in == Input::LEFT_PUSH && event.trans == InputTransition::RELEASE) {
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
        if (offset > 0 && offset < (additive1.samples().size() - 128)) {
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

};
std::array<PartialEditor, (AudioSynthAdditive::partial_table_size / 2) / 128> partialEditors;


struct BankWaveEditor : public Screen {
    int selectedTimepoint = 0;
    int selectedHarmonic = 0;
    bool editingDelay = false;

    BankWaveEditor() : Screen() {}

    void draw() override {
        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen

        main_oled.setCursor(0, 0);
        main_oled.print(selectedTimepoint);
        main_oled.print(" Duration:");
        if (editingDelay) {
            main_oled.setTextColor(colors::cornflowerblue, colors::black);
        }
        else {
            main_oled.setTextColor(colors::white);
        }
        main_oled.print(audio::as_module.bankVoice()[selectedTimepoint].t / 44.f);

        main_oled.setTextColor(colors::white);

        int partial = 0;

        for (int row = 0; row < 1; row++) {
            constexpr auto rowHeight = 112;
            constexpr auto halfHeight = rowHeight / 2;
            int y = row * rowHeight;
            int yh = y + halfHeight;

            main_oled.drawFastHLine(0, yh, 128, colors::darkgrey);

            for (int i = 0; i < AudioSynthOscBank::bankSize; i++) {
                constexpr auto boxWidth = 128 / AudioSynthOscBank::bankSize;
                constexpr auto innerWidth = boxWidth - 2;

                auto & voice = audio::as_module.bankVoice();
                auto amp = voice[selectedTimepoint][i];
                auto phase = voice[selectedTimepoint][i + AudioSynthOscBank::bankSize] / 4294967296.f;

                if (!editingDelay && partial == selectedHarmonic) 
                    main_oled.drawRect(i * boxWidth, y, boxWidth, rowHeight, colors::cornflowerblue);

                int boxHeight = amp * (halfHeight - 1);
                main_oled.fillRect(i * boxWidth + 1, yh - boxHeight, innerWidth, boxHeight, colors::darkorange);

                boxHeight = phase * (halfHeight - 1);
                main_oled.fillRect(i * boxWidth + 1, yh, innerWidth, boxHeight, colors::hotpink);

                partial++;
            }
        }

        dirty = false;
    }

    virtual bool handleInput(InputEvent const& ev) override {
        if (Screen::handleInput(ev)) return true;

        if (ev.trans == InputTransition::RELEASE) {
            if (ev.in == Input::NAV_SOUTH) {
                selectedTimepoint = (selectedTimepoint + 1) % AudioSynthOscBank::nControlPoints;
                sully();
                return true;
            }
            else if ( ev.in == Input::NAV_NORTH) {
                selectedTimepoint--;
                if (selectedTimepoint < 0) {
                    selectedTimepoint = AudioSynthOscBank::nControlPoints - 1;
                }
                sully();
                return true;
            }
            else if (ev.in == Input::NAV_CENTER) {
                editingDelay = false;
                sully();
                return true;
            }
        }
        else if (ev.trans == InputTransition::PRESS) {
            if (ev.in == Input::NAV_CENTER) {
                editingDelay = true;
                sully();
                return true;
            }
        }

        return false;
    }

    virtual void passInputToWidget(InputEvent const& event) override {

        if (editingDelay) {
            if (event.in == Input::LEFT_ROTATE || event.in == Input::RIGHT_ROTATE) {
                int & t = audio::as_module.bankVoice()[selectedTimepoint].t;
                if (event.trans == InputTransition::INCR) {
                    t += 440;
                }
                else {
                    t -= 440;
                    if (t < 440) { t = 440; }
                }
                sully();
            }
            return;
        }

        auto & voice = audio::as_module.bankVoice();
        auto & amp = voice[selectedTimepoint][selectedHarmonic];
        auto & phase = voice[selectedTimepoint][selectedHarmonic + AudioSynthOscBank::bankSize];

        if (event.in == Input::RIGHT_ROTATE) {
            if (event.trans == InputTransition::DECR) {
                amp = em::clamp_incr(0, amp, 1.f, -0.01f);
            }
            else {
                amp = em::clamp_incr(0, amp, 1.f, 0.01f);
            }
        }
        else if (event.in == Input::RIGHT_PUSH && event.trans == InputTransition::RELEASE) {
            amp = 0;
            phase = 0;
        }

        constexpr uint32_t phaseIncr = 4294967296 / 128;

        if (event.in == Input::LEFT_ROTATE) {
            if (event.trans == InputTransition::DECR) {
                phase -= phaseIncr;
                if (phase < 0) {
                    phase = 4294967296;
                }
            }
            else {
                phase += phaseIncr;
                if (phase >= 4294967296) {
                    phase = 0;
                }
            }
        }
        else if (event.in == Input::LEFT_PUSH && event.trans == InputTransition::RELEASE) {
            phase = 0;
        }

        sully();
    }

    virtual void nextWidget() override {
        selectedHarmonic++;
        if (selectedHarmonic >= AudioSynthOscBank::bankSize) selectedHarmonic = 0;
        sully();
    }

    virtual void prevWidget() override { 
        selectedHarmonic--;
        if (selectedHarmonic < 0) selectedHarmonic = AudioSynthOscBank::bankSize - 1;
        sully();
    }

    virtual bool hasScope() override { return true; }

    int offset = 0;

    virtual void drawScope() override {
        float tempBuffer[128];
        oscbank1.previewVoice(tempBuffer, 128);
        display::draw_buffer_in_scope2(tempBuffer);
     }
    

} bankWaveEditor;

// color table for FFT grid display.

struct FFTGrid : public Screen {
    FFTGrid() : Screen() {}

    virtual bool showPerf() override { return false; }

    void draw() override {
        using namespace display;
        using namespace colors;

        if (!dirty) return;

        main_oled.fillScreen(0); // clear the screen
        
        constexpr int binPxWidth = 8;
        constexpr int binPxHeight = 8;

        for (int y = 0; (y + binPxHeight) <= 128; y += binPxHeight){
            for (int x = 0; (x + binPxWidth) <= 128; x += binPxWidth) {
                main_oled.fillRoundRect(x, y, binPxWidth, binPxHeight, 2, (rand() & 0xffff));
            }
        }
        
        dirty = false;
    }

} fftGrid;

static struct ScreenConstructor {
    ScreenConstructor() {
        // link our screens together
        mainScreen.link(&partialEditors[0], East);
        Screen *l = &partialEditors[0];
        int i = 0;
        for (auto & pe : partialEditors) {
            pe.firstPartialOffset = i++ * 128;
            l = l->link(&pe, South);
        }

        partialEditors[0].link(&fftGrid, East); // add in the FFT grid.
        fftGrid.link(&bankWaveEditor, East);


        // link to main graph

        Screen* parent = rootScreen->zipTo(North);
        parent->link(&mainScreen, North);
        
    }
} _screenConstructor;

}