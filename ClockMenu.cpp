#include "ClockMenu.h"
#include "Audio.h"
#include "Globals.h"
#include "Settings.h"
#include "TimeManager.h"
#include "graphics.h"
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

ClockMenu::ClockMenu(Menu* mainMenu)
{
	this->parent = this;
	this->sub_menu = mainMenu;
	mainMenu->parent = this;
}

Menu* ClockMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	// Clock menu consists of
	// - clock face determined by clock face function
	// - Status symbols above
	// - Date below

	auto date_text = TimeManager::the().date_de();
	auto current_time = TimeManager::the().current_time();

	display->firstPage();
	do {
		display->setDrawColor(1);
		uint16_t current_symbol_position = SCREEN_WIDTH;
		// status symbols
		if (WiFi.status() == WL_CONNECTED) {
			display->drawXBMP(current_symbol_position - wifi_symbol_width, 0, wifi_symbol_width,
				wifi_symbol_height, wifi_symbol_bits);
			current_symbol_position -= wifi_symbol_width + SYMBOL_SPACING;
		} else {
			display->drawXBMP(current_symbol_position - nowifi_symbol_width, 0, nowifi_symbol_width,
				nowifi_symbol_height, nowifi_symbol_bits);
			current_symbol_position -= nowifi_symbol_width + SYMBOL_SPACING;
		}
		yield();

		// TODO: display alarm clock symbol if an alarm clock is set

		if (TimeManager::the().time_since_ntp_update() < CLOCKSYNC_SYMBOL_DURATION) {
			display->drawXBMP(current_symbol_position - clocksync_symbol_width, 0,
				clocksync_symbol_width, clocksync_symbol_height,
				clocksync_symbol_bits);
			current_symbol_position -= clocksync_symbol_width + SYMBOL_SPACING;
		}
		yield();

		if (AudioManager::the().is_playing()) {
			display->drawXBMP(current_symbol_position - sound_symbol_width, 0, sound_symbol_width,
				sound_symbol_height, sound_symbol_bits);
			current_symbol_position -= sound_symbol_width + SYMBOL_SPACING;
		}
		yield();

		uint64_t time = micros64();
		current_clock_face(display, &current_time, 0, 0, display->getDisplayWidth(),
			display->getDisplayHeight());
		yield();
		display->setFont(TINY_FONT);
		display->drawUTF8(LEFT_TEXT_MARGIN, SCREEN_HEIGHT, date_text.c_str());
		uint64_t after_time = micros64();
		yield();

		if (eeprom_settings.show_timing) {
			uint64_t total_time = after_time - time;
			uint32_t cycles = microsecondsToClockCycles(total_time);
			char timing_text[22];
			snprintf_P(timing_text, sizeof(timing_text), PSTR("%02.3fm %10d"), total_time / 1000.0d, cycles);

			display->drawUTF8(SCREEN_WIDTH - LEFT_TEXT_MARGIN - 70, SCREEN_HEIGHT,
				timing_text);
		}

		yield();
	} while (display->nextPage());

	return this;
}

bool ClockMenu::should_refresh(uint16_t delta_millis)
{
	this->last_display_update += delta_millis;
	if (this->last_display_update > CLOCK_UPDATE_INTERVAL) {
		this->last_display_update = 0;
		return true;
	}
	return false;
}

Menu* ClockMenu::handle_button(uint8_t buttons)
{
	if (buttons & BUTTON_RIGHT) {
		return this->sub_menu;
	}
	return this;
}
