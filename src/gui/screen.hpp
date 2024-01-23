#pragma once
#include "../display.h"
#include "../util/emmath.h"
#include "colors.hpp"
#include <tuple>
#include <functional>
#include <audio/Control.hpp>

namespace gui {


enum class WidgetInput {
    LEFT_DECR, // stick turns
    LEFT_INCR,
    RIGHT_DECR,
    RIGHT_INCR,

    LEFT_PUSH, // stick press and releases
    LEFT_REL,
    RIGHT_PUSH,
    RIGHT_REL, 

    NULL_INPUT
};


// Receives input from the encoders and their buttons.
class Widget {
protected:
    em::ivec topLeft;
public:
    virtual void draw(bool focused);

    void position(em::ivec const& tl) {
        topLeft = tl;
    }

    void link(Widget & nextWidget) {
        next = &nextWidget;
        nextWidget.prev = this;
    }

    virtual void handleInput(WidgetInput event) {};

    virtual int height() { return 0; }
    virtual int width() { return 0; }

    virtual ~Widget() {};

    Widget *prev = nullptr, *next = nullptr;
};


/**
 * Display two numerical controls with labels. They must be of the same type.
*/
template <typename VALTYPE>
class DualNumericalWidget : public Widget {
public:
    using valtype = VALTYPE;
    using incr_func = std::function<valtype(const valtype)>;

    static valtype _default_incr(const valtype) { return 1; }

protected:
  const char *aLabel, *bLabel;
  valtype aVal, bVal;
  std::tuple<valtype, valtype> aLimits{0, 127}, bLimits{0, 127};
  incr_func aIncr = _default_incr, bIncr = _default_incr; // increment functions

  audio::Control<valtype> *aControl = nullptr, *bControl = nullptr;

  int aLabelWidth = 0, bLabelWidth = 0;
  valtype aIncrScale = 1, bIncrScale = 1;

public:
    explicit DualNumericalWidget(const char *aLabel, const char *bLabel, audio::Control<valtype> &aControl, audio::Control<valtype> &bControl)
        : Widget(), aLabel(aLabel), bLabel(bLabel), aVal(*aControl), bVal(*bControl), aControl(&aControl), bControl(&bControl)
    {
        calcLabelLength();
    }

    DualNumericalWidget(const char *aLabel, const char *bLabel, valtype const& a, valtype const& b)
        : Widget(), aLabel(aLabel), bLabel(bLabel), aVal(a), bVal(b)
    {
        
        calcLabelLength();
    }

    void calcLabelLength() {
        aLabelWidth = strlen(aLabel) * 6;
        bLabelWidth = strlen(bLabel) * 6;
    }

    virtual int height() { 
        return 16 + // two lines of characters
        1 + // spacing between
        2 + // padding
        2; // border
    }

    /// @brief Set the numerical limits for the values.
    void setLimits(valtype aLow, valtype aHigh, valtype bLow, valtype bHigh) {
        aLimits = std::make_tuple(aLow, aHigh);
        bLimits = std::make_tuple(bLow, bHigh);
    }

    void setIncrements(incr_func a, incr_func b) {
        aIncr = a; 
        bIncr = b;
    }

    void draw(bool focused) override {
        using namespace display;
        using namespace colors;

        auto bg = focused ? colors::white : colors::black;
        auto fg = focused ? colors::black : colors::white;

        main_oled.fillRect(topLeft.x, topLeft.y, 128, height(), bg);

        if (!focused) {
            main_oled.drawRect(topLeft.x, topLeft.y, 128, height(), fg);
        }
        
        main_oled.setTextSize(1);
        main_oled.setTextColor(fg, bg);

        const auto topline = topLeft.y + 2;
        const auto botline = topline + 8 ;

        main_oled.setCursor(2, topline);
        main_oled.print("\xda");
        main_oled.print(aLabel);

        main_oled.setCursor(2, botline);
        main_oled.print("\xc0");
        main_oled.setCursor(2 + 6, botline + 1);
        main_oled.print(a());
        
        main_oled.setCursor(64 + 2, topline);
        main_oled.print("\xda");
        main_oled.print(bLabel);

        main_oled.setCursor(64 + 2, botline);
        main_oled.print("\xc0");
        main_oled.setCursor(64 + 2 + 6, botline + 1);
        main_oled.print(b());

        //main_oled.drawFastVLine(64, topLeft.y, height(), fg); // kinda hate how this looks.
    }

    valtype const& a() {
        if (aControl) return **aControl;
        return aVal;
    }

    valtype const& b() {
        if (bControl) return **bControl;
        return bVal;
    }

    void setA(valtype const& newValue) {
        if (aControl) {
            aControl->set(newValue);
        }
        else {
            aVal = newValue;
        }
    }

    void setB(valtype const& newValue) {
        if (bControl) {
            bControl->set(newValue);
        }
        else {
            bVal = newValue;
        }
    }

    virtual void handleInput(WidgetInput event) {
        switch (event) {
        case WidgetInput::LEFT_DECR:
            setA(em::clamp_incr(std::get<0>(aLimits), a(), std::get<1>(aLimits), aIncrScale * -aIncr(a())));
            break;
        case WidgetInput::LEFT_INCR:
            setA(em::clamp_incr(std::get<0>(aLimits), a(), std::get<1>(aLimits), aIncrScale * aIncr(a())));
            break;
        case WidgetInput::RIGHT_DECR:
            setB(em::clamp_incr(std::get<0>(bLimits), b(), std::get<1>(bLimits), bIncrScale * -bIncr(b())));
            break;
        case WidgetInput::RIGHT_INCR:
            setB(em::clamp_incr(std::get<0>(bLimits), b(), std::get<1>(bLimits), bIncrScale * bIncr(b())));
            break;
        case WidgetInput::LEFT_PUSH:
            aIncrScale = 10;
            break;
        case WidgetInput::LEFT_REL:
            aIncrScale = 1;
            break;
        case WidgetInput::RIGHT_PUSH:
            bIncrScale = 10;
            break;
        case WidgetInput::RIGHT_REL:
            bIncrScale = 1;
            break;
        default:
            break;
        }
    }
};

using Direction = int;

constexpr Direction North = 0;
constexpr Direction East = 1;
constexpr Direction South = 2;
constexpr Direction West = 3;

constexpr Direction opposite_direction(Direction dir) {
    switch (dir) {
    case North:
        return South;
    case South:
        return North;
    case East: 
        return West;
    case West:
        return East;
    }

    return -1;
}

/**
 * A single, stateful instance of a particular screen.
*/
class Screen {
protected:

    /// @brief Is the screen "dirty" and in need of a redraw?
    bool dirty = true;

    /// @brief Widget currently receiving focus.
    Widget *focusedWidget = nullptr;

    std::array<Screen*, 4> neighbors {nullptr, nullptr, nullptr, nullptr};

public:

    /// @brief Draw the contents of the screen to the actual OLED.
    virtual void draw();
    
    /// @brief Set the dirty flag to true.
    void sully() { dirty = true; }

    /// @brief Is the given widget currently focused?
    bool focused(Widget const& w) { return focusedWidget == &w; }
    
    /// @brief Link a new screen to this one in the given direction.
    void link(Screen* neighbor, Direction dir) {
        neighbors[dir] = neighbor;
        neighbor->neighbors[opposite_direction(dir)] = this;
    }

    /// @brief Get the neighboring screen in the given direction, or nullptr if none exists.
    Screen* nextScreen(Direction dir) {
        return neighbors[dir];
    }

    /// @brief Get the farthest screen reachable in the given direction. Returns this screen if no neighbor exists. 
    Screen* zipTo(Direction dir) {
        Screen *s = this;
        while (Screen *n = s->nextScreen(dir)) {
            s = n;
        }
        return s;
    }

    /// @brief Scroll to the next widget.
    void nextWidget() {
        if (!focusedWidget) return;
        if (focusedWidget->next) {
            focusedWidget->draw(false);
            focusedWidget = focusedWidget->next;
            focusedWidget->draw(true);
        }
    }

    /// @brief Scroll to the previous widget.
    void prevWidget() {
        if (!focusedWidget) return;
        if (focusedWidget->prev) {
            focusedWidget->draw(false);
            focusedWidget = focusedWidget->prev;
            focusedWidget->draw(true);
        }
    }

    /// @brief Pass inputs to the focused widget.
    virtual void handleInput(WidgetInput const& event) {
        if (!focusedWidget) return;
        if (focusedWidget) {
            focusedWidget->handleInput(event);
            focusedWidget->draw(true); // we must be focused if we're getting input
        }
    }

    virtual ~Screen() {};
};

extern Screen* rootScreen;
extern Screen* activeScreen;

Screen* go_to_screen(Screen *s);

}