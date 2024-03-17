#pragma once

#include "Menu.h"

constexpr double FPS = 2;
constexpr uint16_t MILLIS_PER_FRAME = static_cast<uint16_t>((1.0 / FPS) * 1000.0);

class VideoPlayer : public Menu {
public:
	virtual Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	virtual bool should_refresh(uint16_t delta_millis) override;
	virtual Menu* handle_button(uint8_t buttons) override;

private:
	uint16_t time_since_last_update { MILLIS_PER_FRAME };
	size_t current_frame { 0 };
};
