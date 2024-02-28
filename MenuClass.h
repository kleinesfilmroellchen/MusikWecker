/**
   Menu class file header.
*/
#pragma once

#include "defs.h"
#include <U8g2lib.h>

class Menu {
public:
    Menu* parent = nullptr;
    /** Called when the menu should be drawn. Delta is the time since the last draw call. Returns the menu that is now active. */
    virtual Menu* drawMenu(T_DISPLAY* disp, uint16_t deltaMillis) = 0;
    /** Returns whether to issue another draw call. Delta is the time since the last refresh check. */
    virtual bool shouldRefresh(uint16_t deltaMillis) = 0;
    /** Called when button state change occurrs. The parameter is the AND of all buttons currently pressed. Returns the menu that is now active. */
    virtual Menu* handleButton(uint8_t buttons) = 0;
};
