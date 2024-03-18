#include "VideoPlayer.h"
#include "Audio.h"
#include "DisplayUtils.h"
#include "bad_apple.h"
#include <Arduino.h>
#include <vector>
// #include <TIFF_G4.h>

// void tiff_callback(TIFFDRAW* tiff_draw_data)
// {
// 	Display* display = static_cast<Display*>(tiff_draw_data->pUser);
// 	auto y = tiff_draw_data->y;
// 	auto x = (display->getWidth() - tiff_draw_data->iScaledWidth) / 2;
// 	display->drawXBM(x, y, tiff_draw_data->iScaledWidth, 1, tiff_draw_data->pPixels);
// 	yield();
// 	display->setFont(TINY_FONT);
// 	draw_string(display, "hello, world", 0);
// }

Menu* VideoPlayer::draw_menu(Display* display, uint16_t delta_millis)
{
	auto* current_frame_data_progmem = frames[current_frame];
	auto current_frame_size = frame_sizes[current_frame];

	// TIFFG4 tiff_loader;
	// auto result = tiff_loader.openTIFF(current_frame_data, current_frame_size, tiff_callback);
	// tiff_loader.setDrawParameters(1., TIFF_PIXEL_1BPP, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, nullptr);
	// tiff_loader.setUserPointer(static_cast<void*>(display));
	// if (result != 1)
	// 	result = tiff_loader.decode();
	auto x = (display->getWidth() - IMAGE_WIDTH) / 2;

	std::vector<uint8_t> current_frame_data;
	current_frame_data.resize(current_frame_size);
	memcpy_P(current_frame_data.data(), current_frame_data_progmem, current_frame_size);

	display->firstPage();
	do {
		yield();
		display->drawXBM(x, 0, IMAGE_WIDTH, IMAGE_HEIGHT, current_frame_data.data());
		yield();
		// char frame_info_text[256] {};
		// snprintf_P(frame_info_text, sizeof(frame_info_text),
		// 	PSTR("frame #%ld\npos %f"),
		// 	current_frame, AudioManager::the().current_position());
		// yield();
		// display->setFont(TINY_FONT);
		// yield();
		// draw_string(display, frame_info_text, 0);
		// yield();
	} while (display->nextPage());
	// tiff_loader.close();

	return this;
}

bool VideoPlayer::should_refresh(uint16_t delta_millis)
{
	auto new_frame = static_cast<size_t>(AudioManager::the().current_position() * FPS) % (sizeof(frames) / sizeof(*frames));
	auto should_refresh = new_frame != current_frame;

	current_frame = new_frame;
	return should_refresh;
}

Menu* VideoPlayer::handle_button(uint8_t buttons)
{
	if (buttons & BUTTON_LEFT) {
		return this->parent;
	}
	if (buttons & BUTTON_RIGHT) {
		String bad_apple("Bad Apple.flac");
		AudioManager::the().play(bad_apple);
	} 
	return this;
}
