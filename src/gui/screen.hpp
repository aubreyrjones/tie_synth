#pragma once
#include "../display.h"
#include "../util/emmath.h"

namespace gui {

class Widget {
protected:
    em::ivec position;
    em::ivec size;
public:
    virtual void draw(bool focused);
};

/**
 * A single, stateful instance of a particular screen.
*/
class Screen {
protected:

    /// @brief Is the screen "dirty" and in need of a redraw?
    bool dirty = true;

    

public:

    /// @brief Draw the contents of the screen to the actual OLED.
    virtual void draw();
};

extern Screen* rootScreen;
extern Screen* activeScreen;

}