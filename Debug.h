#pragma once

#include "DisplayUtils.h"
#include "Settings.h"
#include "defs.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <U8g2lib.h>

// display
extern T_DISPLAY display;

template <typename Printable>
inline void debug_print(Printable text)
{
    Serial.println(text);

    if (eeprom_settings.show_timing) {
        if (display.getCursorY() >= display.getDisplayHeight()) {
            display.clear();
            display.setCursor(LINE_HEIGHT, 0);
        }

        display.firstPage();
        do {
            display.setDrawColor(1);
            display.setFont(TINY_FONT);
            display.println(text);

            yield();
        } while (display.nextPage());
    }
}