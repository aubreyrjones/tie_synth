#pragma once
#include "../display.h"
#include "../util/emmath.h"

namespace gui {

class Widget {
protected:
public:
    virtual void draw(em::ivec const& topLeft, bool focused);

    void link(Widget & nextWidget) {
        next = &nextWidget;
        nextWidget.prev = this;
    }

    virtual int height() { return 0; }
    virtual int width() { return 0; }

    virtual ~Widget() {};

    Widget *prev = nullptr, *next = nullptr;
};


/**
 * Display two numerical controls with labels.
*/
class DualNumericalWidget : public Widget {
protected:
  const char *aLabel, *bLabel;
  byte aVal, bVal;

public:
    DualNumericalWidget(const char *aLabel, const char *bLabel, byte a, byte b)
        : Widget(), aLabel(aLabel), bLabel(bLabel), aVal(a), bVal(b) {}

    virtual int height() { return 12; }

  void draw(em::ivec const& topLeft, bool focused) override;
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
            focusedWidget = focusedWidget->next;
            sully();
        }
    }

    void prevWidget() {
        if (focusedWidget->prev) {
            focusedWidget = focusedWidget->prev;
            sully();
        }
    }

    virtual ~Screen() {};
};

extern Screen* rootScreen;
extern Screen* activeScreen;

}