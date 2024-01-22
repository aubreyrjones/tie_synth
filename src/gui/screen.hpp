#pragma once
#include "../display.h"
#include "../util/emmath.h"
#include "colors.hpp"

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
 * Display two numerical controls with labels.
*/
template <typename VAL_TYPE>
class DualNumericalWidget : public Widget {
public:
    using valtype = VAL_TYPE;

protected:
  const char *aLabel, *bLabel;
  valtype aVal, bVal;

  int aLabelWidth = 0, bLabelWidth = 0;

public:
    DualNumericalWidget(const char *aLabel, const char *bLabel, valtype a, valtype b)
        : Widget(), aLabel(aLabel), bLabel(bLabel), aVal(a), bVal(b) {
            aLabelWidth = strlen(aLabel) * 8;
            bLabelWidth = strlen(bLabel) * 8;
        }

    virtual int height() { return 12; }

    void draw(bool focused) override {
        using namespace display;
        using namespace colors;

        auto bg = focused ? colors::white : colors::black;
        auto fg = focused ? colors::black : colors::white;

        main_oled.fillRect(topLeft.x, topLeft.y, 128, 12, bg);

        if (!focused) {
            main_oled.drawRect(topLeft.x, topLeft.y, 128, 12, fg);
        }
        else {
            main_oled.drawRect(topLeft.x, topLeft.y, 128, 12, bg);
        }

        
        main_oled.setTextSize(1);
        main_oled.setTextColor(fg, bg);


        main_oled.setCursor(2, topLeft.y + 2);
        main_oled.print(aLabel);
        main_oled.print(":");

        main_oled.setCursor(64 - 24, topLeft.y + 2);
        main_oled.print(aVal, DEC);
        
        main_oled.setCursor(64, topLeft.y + 2);
        main_oled.print(bLabel);
        main_oled.print(":");

        main_oled.setCursor(128 - 24, topLeft.y + 2);
        main_oled.print(bVal, DEC);
    }


    virtual void handleInput(WidgetInput event) {
        switch (event) {
        case WidgetInput::LEFT_DECR:
            em::clamp_incr(0, aVal, 127, -1);
            break;
        case WidgetInput::LEFT_INCR:
            em::clamp_incr(0, aVal, 127, 1);
            break;
        case WidgetInput::RIGHT_DECR:
            em::clamp_incr(0, bVal, 127, -1);
            break;
        case WidgetInput::RIGHT_INCR:
            em::clamp_incr(0, bVal, 127, 1);
            break;
        default:
            break;
        }
    }

};

/**
 * A single, stateful instance of a particular screen.
*/
class Screen {
protected:

    /// @brief Is the screen "dirty" and in need of a redraw?
    bool dirty = true;

    /// @brief Widget currently receiving focus.
    Widget *focusedWidget;

public:

    /// @brief Draw the contents of the screen to the actual OLED.
    virtual void draw();
    
    /// @brief Set the dirty flag to true.
    void sully() { dirty = true; }

    /// @brief Is the given widget currently focused?
    /// @param w 
    /// @return 
    bool focused(Widget const& w) { return focusedWidget == &w; }
    
    /// @brief Scroll to the next widget.
    void nextWidget() {
        if (focusedWidget->next) {
            focusedWidget->draw(false);
            focusedWidget = focusedWidget->next;
            focusedWidget->draw(true);
        }
    }

    void prevWidget() {
        if (focusedWidget->prev) {
            focusedWidget->draw(false);
            focusedWidget = focusedWidget->prev;
            focusedWidget->draw(true);
        }
    }

    virtual void handleInput(WidgetInput const& event) {
        if (focusedWidget) {
            focusedWidget->handleInput(event);
            focusedWidget->draw(true); // we must be focused if we're getting input
        }
    }

    virtual ~Screen() {};
};

extern Screen* rootScreen;
extern Screen* activeScreen;

}