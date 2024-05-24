#include "TimeFormatMenu.h"
#include "Globals.h"
#include "Settings.h"
#include "graphics.h"
#include "strings.h"

Menu* TimeFormatMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	this->is_dirty = false;
	const auto width = display->getDisplayWidth();

	display->firstPage();
	display->setDrawColor(2);
	display->setBitmapMode(1);
	display->setFont(MAIN_FONT);
	do {
		for (uint16_t i = 0; i < 4; ++i) {
			const auto* display_string = time_formats[i];
			if (menu_idx == i)
				display->drawBox(0, position_of_line(i), width, LINE_HEIGHT);
			draw_string(display, display_string, i);

			// when displaying a time format option, check whether that option is enabled.
			// when displaying the "show seconds" option, check whether the show seconds switch is enabled.
			if ((i < 3 && static_cast<uint8_t>(eeprom_settings.clock_settings.time_format) == i)
				|| (i == 3 && eeprom_settings.clock_settings.show_seconds))
				display->drawXBMP(display->getDisplayWidth() - checkmark_symbol_width, position_of_line(i),
					checkmark_symbol_width, checkmark_symbol_height, checkmark_symbol_bits);

			yield();
		}

		yield();
	} while (display->nextPage());

	return this;
}

bool TimeFormatMenu::should_refresh(uint16_t delta_millis)
{
	return is_dirty;
}

Menu* TimeFormatMenu::handle_button(uint8_t buttons)
{
	if (buttons != 0)
		this->is_dirty = true;

	// back and enter button logic
	if (buttons & BUTTON_LEFT) {
		return this->parent;
	}

	if (buttons & BUTTON_RIGHT) {
		if (menu_idx < 3) {
			eeprom_settings.clock_settings.time_format = static_cast<TimeFormat>(menu_idx);
		} else {
			eeprom_settings.clock_settings.show_seconds = !eeprom_settings.clock_settings.show_seconds;
		}
		save_settings();
	}

	// down and up button (next/previous element) logic
	if (buttons & BUTTON_DOWN)
		++menu_idx;
	else if (buttons & BUTTON_UP)
		--menu_idx;

	menu_idx = (menu_idx + 4) % 4;

	// happens only for up and down buttons
	return this;
}
