#include "VideoPlayer.h"
#include "DisplayUtils.h"
#include "bad_apple.h"
#include <TIFF_G4.h>

void tiff_callback(TIFFDRAW* tiff_draw_data)
{
	Display* display = static_cast<Display*>(tiff_draw_data->pUser);
	// auto y = tiff_draw_data->y;
	// auto x = (display->getWidth() - tiff_draw_data->iScaledWidth) / 2;
	// display->drawXBM(x, y, tiff_draw_data->iScaledWidth, 1, tiff_draw_data->pPixels);
	// yield();
	display->setFont(TINY_FONT);
	draw_string(display, "hello, world", 0);
}

Menu* VideoPlayer::draw_menu(Display* display, uint16_t delta_millis)
{
	auto* current_frame_data = frames[current_frame];
	auto current_frame_size = frame_sizes[current_frame];

	TIFFG4 tiff_loader;
	auto result = tiff_loader.openTIFF(current_frame_data, current_frame_size - 1, tiff_callback);
	tiff_loader.setUserPointer(static_cast<void*>(display));
	// if (result != 0)
	// 	result = tiff_loader.decode();

	display->firstPage();
	do {
		char frame_info_text[256] {};
		snprintf_P(frame_info_text, sizeof(frame_info_text),
			PSTR("frame #%ld\nsize %lx\npointer %x\ndecode result %d"),
			current_frame, current_frame_size, current_frame_data, result);
		display->setFont(TINY_FONT);
		draw_string(display, frame_info_text, 0);
	} while (display->nextPage());
	tiff_loader.close();

	return this;
}

bool VideoPlayer::should_refresh(uint16_t delta_millis)
{
	time_since_last_update += delta_millis;
	if (time_since_last_update >= MILLIS_PER_FRAME) {
		time_since_last_update -= MILLIS_PER_FRAME;
		current_frame = (current_frame + 1) % (sizeof(frames) / sizeof(*frames));
		return true;
	}
	return false;
}

Menu* VideoPlayer::handle_button(uint8_t buttons)
{
	if (buttons & BUTTON_LEFT) {
		return this->parent;
	}
	return this;
}
