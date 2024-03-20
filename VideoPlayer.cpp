#include "VideoPlayer.h"
#include "Audio.h"
#include "DisplayUtils.h"
#include "SRLV.h"
#include "bad_apple.h"
#include <Arduino.h>
#include <umm_malloc/umm_heap_select.h>
#include <user_interface.h>
#include <vector>

constexpr size_t ROW_SIZE = IMAGE_WIDTH / 8 + 1;

Menu* VideoPlayer::draw_menu(Display* display, uint16_t delta_millis)
{
	// the 1.3K of main heap we might have left when entering this function are not enough.
	HeapSelectIram iram;

	auto* current_frame_data_progmem = frames[current_frame];
	auto current_frame_size = frame_sizes[current_frame];
	// std::vector<uint8_t> current_frame_data;
	// current_frame_data.resize(current_frame_size);
	// memcpy_P(current_frame_data.data(), current_frame_data_progmem, current_frame_size);

	yield();
	// auto start_time = micros();
	auto decompressed = SRLV::decompress({ current_frame_data_progmem, current_frame_size }, IMAGE_WIDTH * IMAGE_HEIGHT, last_frame);
	// auto end_time = micros();
	yield();

	auto x = (display->getWidth() - IMAGE_WIDTH) / 2;
	display->setDrawColor(1);
	display->firstPage();
	do {
		yield();
		// auto draw_start = micros();
		display->drawXBM(x, 0, IMAGE_WIDTH, IMAGE_HEIGHT, decompressed.data());
		// auto draw_end = micros();
		// yield();
		// char frame_info_text[256] {};
		// snprintf_P(frame_info_text, sizeof(frame_info_text),
		// 	PSTR("f %ld = %.1f pos %.2f\nsr %ld sn %ld\ndec  %5ld\ndraw %5ld\nheap %d"),
		// 	current_frame, AudioManager::the().current_position() * FPS, AudioManager::the().current_position(), AudioManager::the().sample_rate(), AudioManager::the().played_sample_count(), end_time - start_time, draw_end - draw_start, system_get_free_heap_size());
		// yield();
		// display->setFont(TINY_FONT);
		// yield();
		// display->setDrawColor(2);
		// draw_string(display, frame_info_text, 0);
		// yield();
	} while (display->nextPage());

	yield();
	last_frame = decompressed;

	return this;
}

bool VideoPlayer::should_refresh(uint16_t delta_millis)
{
	if (!AudioManager::the().is_playing()) {
		time_since_last_frame += delta_millis;
		if (time_since_last_frame >= MILLIS_PER_FRAME) {
			time_since_last_frame -= MILLIS_PER_FRAME;
			current_frame = (current_frame + 1) % (sizeof(frames) / sizeof(*frames));
			return true;
		} else {
			return false;
		}
	}
	// FIXME: magic number here is a hack to fix a consistent A/V desync. may be 44.1/48 confusion but idk.
	auto new_frame = static_cast<size_t>(AudioManager::the().current_position() * FPS * (219.0 / 224.0)) % (sizeof(frames) / sizeof(*frames));
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
		String bad_apple(F("/Bad Apple.flac"));
		AudioManager::the().play(bad_apple);
	}
	return this;
}
