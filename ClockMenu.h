/** Clock menu header. */

#pragma once

#include "ClockFaces.h"
#include "Definitions.h"
#include "Menu.h"
#include <AceTime.h>
#include <NTPClient.h>

/**
 * Menu that can displays the clock.
 */
class ClockMenu : public Menu {
private:
	Menu* sub_menu;
	uint32_t last_display_update = 0;
	uint64_t time_of_second_rollover = 0;
	ace_time::acetime_t last_timestamp = 0;

public:
	/** The Clock menu takes a reference to the ntp client responsible for time retrieval, the time zone it should display time in, and the main menu. */
	ClockMenu(Menu* mainMenu);
	Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	bool should_refresh(uint16_t delta_millis) override;
	Menu* handle_button(uint8_t buttons) override;
};
