#pragma once
#include "../display.h"
#include "../util/emmath.h"
#include "colors.hpp"
#include <tuple>
#include <functional>
#include <audio/Control.hpp>

namespace gui {


enum class WidgetInput {
    LEFT_DECR, // encoder turns
    LEFT_INCR,
    RIGHT_DECR,
    RIGHT_INCR,

    LEFT_PUSH, // encoder press and releases
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

    virtual void position(em::ivec const& tl) {
        topLeft = tl;
    }

    virtual Widget& link(Widget & nextWidget) {
        next = &nextWidget;
        nextWidget.prev = this;
        return nextWidget;
    }

    virtual void handleInput(WidgetInput event) {};

    virtual int height() { return 0; }
    virtual int width() { return 0; }

    virtual ~Widget() {};

    Widget *prev = nullptr, *next = nullptr;
};


template <typename VALTYPE>
class NumericalWidget : public Widget {
public:
    using valtype = VALTYPE;
    using incr_func = std::function<valtype(const valtype)>;

    static valtype _default_incr(const valtype) { return 1; }

    static constexpr int fontSize = 1;
    static constexpr int fontWidth = 6 * fontSize;
    static constexpr int fontHeight = 8 * fontSize;

protected:
  const char *aLabel;

  incr_func aIncr = _default_incr; // increment function

  audio::Control<valtype> *aControl = nullptr;

  int aLabelWidth = 0;
  valtype aIncrScale = 1;

public:
    explicit NumericalWidget(audio::Control<valtype> &control)
        : Widget(), aLabel(control.name()), aControl(&control)
    {
        calcLabelLength();
    }

    void calcLabelLength() {
        aLabelWidth = strlen(aLabel) * (6 * fontSize);
    }

    virtual int height() { 
        return (2 * fontHeight) + // two lines of characters
        4 + // spacing between
        2 + // padding
        2; // border
    }

    void setIncrements(incr_func a) {
        aIncr = a;
    }

    void draw(bool focused) override {
        using namespace display;
        using namespace colors;

        auto bg = focused ? colors::white : colors::black;
        auto fg = focused ? colors::black : colors::white;

        main_oled.setTextSize(fontSize);
        main_oled.setTextColor(fg, bg);

        const auto topline = topLeft.y + 2;
        const auto botline = topline + fontHeight;
        const auto leftText = topLeft.x + 2;
        

        main_oled.setCursor(leftText, topline);
        main_oled.print("\xda");
        main_oled.print(aLabel);

        main_oled.setCursor(leftText, botline);
        main_oled.print("\xc0\x12");
        main_oled.setCursor(leftText + (2 * fontWidth) + 2, botline + 4);
        main_oled.print(a());
    }

    valtype const& a() {
        return **aControl;
    }

    virtual void handleInput(WidgetInput event) {
        switch (event) {
        case WidgetInput::LEFT_DECR:
        case WidgetInput::RIGHT_DECR:
            aControl->adjust(false, aIncrScale * aIncr(a()));
            break;
        case WidgetInput::RIGHT_INCR:
        case WidgetInput::LEFT_INCR:
            aControl->adjust(true, aIncrScale * aIncr(a()));
            break;
        case WidgetInput::LEFT_PUSH:
        case WidgetInput::RIGHT_PUSH:
            aIncrScale = 10;
            break;
        case WidgetInput::LEFT_REL:
        case WidgetInput::RIGHT_REL:
            aIncrScale = 1;
            break;
        default:
            break;
        }
    }
};

template <typename VALTYPE>
class DualNumericalWidget : public Widget {
public:
    using valtype = VALTYPE;
    using incr_func = std::function<valtype(const valtype)>;

    static valtype _default_incr(const valtype) { return 1; }

protected:
    NumericalWidget<valtype> left, right;

public:

    DualNumericalWidget(audio::Control<valtype> &aControl, audio::Control<valtype> &bControl) :
        Widget(), left(aControl), right(bControl) {
    }

    void setIncrements(incr_func a, incr_func b) {
        left.setIncrements(a);
        right.setIncrements(b);
    }

    int height() override {
        return std::max(left.height(), right.height());
    }

    void position(em::ivec const& tl) override {
        Widget::position(tl);
        left.position(tl);
        right.position(tl + em::ivec{64, 0});
    }

    void draw(bool focused) override {
        using namespace display;
        using namespace colors;

        auto bg = focused ? colors::white : colors::black;

        main_oled.fillRect(topLeft.x, topLeft.y, 128, height(), bg);

        left.draw(focused);
        right.draw(focused);
    }

    virtual void handleInput(WidgetInput event) {
        switch (event) {
        case WidgetInput::LEFT_DECR:
        case WidgetInput::LEFT_INCR:
        case WidgetInput::LEFT_PUSH:
        case WidgetInput::LEFT_REL:
            left.handleInput(event);
            break;
        case WidgetInput::RIGHT_DECR:
        case WidgetInput::RIGHT_INCR:
        case WidgetInput::RIGHT_PUSH:
        case WidgetInput::RIGHT_REL:
            right.handleInput(event);
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

    void flowWidgets(em::ivec const& tl, Widget* firstWidget);
    void drawWidgets(Widget* firstWidget);

    virtual ~Screen() {};
};

extern Screen* rootScreen;
extern Screen* activeScreen;

Screen* go_to_screen(Screen *s);

}