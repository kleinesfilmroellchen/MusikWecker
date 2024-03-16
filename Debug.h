#pragma once

#include "Definitions.h"
#include "DisplayUtils.h"
#include "Globals.h"
#include "Settings.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <U8g2lib.h>

template <typename Printable>
inline void debug_print(Printable text)
{
#if USE_SERIAL
	Serial.println(text);
#endif

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
