#pragma once

#include "Menu.h"

constexpr double FPS = 8;
constexpr uint16_t MILLIS_PER_FRAME = static_cast<uint16_t>((1.0 / FPS) * 1000.0);
constexpr int IMAGE_WIDTH = 80;
constexpr int IMAGE_HEIGHT = 64;

class VideoPlayer : public Menu {
public:
	virtual Menu* draw_menu(Display* display, uint16_t delta_millis) override;
	virtual bool should_refresh(uint16_t delta_millis) override;
	virtual Menu* handle_button(uint8_t buttons) override;

private:
	size_t current_frame { 0 };
	// used when not a/v syncing
	uint32_t time_since_last_frame { 0 };

	std::vector<uint8_t> last_frame {};
};
