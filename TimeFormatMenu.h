#pragma once

#include "Definitions.h"
#include "OptionsMenu.h"

class TimeFormatMenu : public Menu {
private:
	bool is_dirty { true };
	uint8_t menu_idx { 0 };

public:
	virtual Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	virtual bool should_refresh(uint16_t delta_millis) override;
	virtual Menu* handle_button(uint8_t buttons) override;
};
