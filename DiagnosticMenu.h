#pragma once

#include "Menu.h"
#include <stdint.h>

enum class DiagnosticPage : uint8_t {
	Time,
	FileSystem,
	__Count,
};

class DiagnosticMenu : public Menu {
public:
	virtual Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	virtual bool should_refresh(uint16_t delta_millis) override;
	virtual Menu* handle_button(uint8_t buttons) override;

private:
	DiagnosticPage current_page = DiagnosticPage::Time;
	bool dirty = true;
	uint32_t time_of_last_ntp = 0;
};
