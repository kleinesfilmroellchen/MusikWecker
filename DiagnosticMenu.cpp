#include "DiagnosticMenu.h"
#include "Audio.h"
#include "DisplayUtils.h"
#include "Globals.h"
#include "NTPClient.h"
#include "PrintString.h"
#include "TimeManager.h"
#include "string_constants.h"
#include <AceTime.h>
#include <SdFat.h>
#include <WString.h>

DiagnosticPage operator+(DiagnosticPage const& self, uint8_t a)
{
	return static_cast<DiagnosticPage>(
		(static_cast<uint8_t>(self) + a) % static_cast<uint8_t>(DiagnosticPage::__Count));
}

DiagnosticPage operator-(DiagnosticPage const& self, uint8_t a)
{
	return static_cast<DiagnosticPage>(
		(static_cast<uint8_t>(self) - a) % static_cast<uint8_t>(DiagnosticPage::__Count));
}

Menu* DiagnosticMenu::draw_menu(Display* display, uint16_t delta_millis)
{
	this->dirty = false;
	display->firstPage();
	do {
		// FIXME: use std::format once Arduino can C++20
		switch (this->current_page) {
		case DiagnosticPage::Time: {
			this->dirty = true;

			String zone_name = TimeManager::the().zone_name();

			char time_info_text[128];
			snprintf_P(
				time_info_text, sizeof(time_info_text),
				PSTR("unix %ds\ntz %s offset %05ds\nserver %s\nlast sync %d (synced "
					 "%d)"),
				TimeManager::the().epoch_time(), zone_name.c_str(), TimeManager::the().current_time().timeOffset(),
				TEMP_TIME_SERVER, millis() - time_of_last_ntp, TimeManager::the().has_network_time());
			display->setFont(TINY_FONT);
			draw_string(display, time_info_text, 0);
			break;
		}

		case DiagnosticPage::FileSystem: {
			this->dirty = true;

			auto capacity_kib = static_cast<uint64_t>(card.vol()->clusterCount() / 1024) * card.vol()->bytesPerCluster();
			auto free_kib = static_cast<uint64_t>(card.vol()->freeClusterCount() / 1024) * card.vol()->bytesPerCluster();
			auto sector_size = card.vol()->bytesPerCluster() / card.vol()->sectorsPerCluster();
			auto cluster_size = card.vol()->bytesPerCluster();
			auto sd_numeric_type = card.card()->type() % 4;
			String sd_type_name = FPSTR(sd_types_array[sd_numeric_type]);

			char file_system_info_text[256] {};
			snprintf_P(file_system_info_text, sizeof(file_system_info_text),
				PSTR("sd type %s\nerror %d payload %d\n%lldKi cap %lldKi "
					 "free\nFAT%d: %d secsz %d clusz"),
				sd_type_name, card.sdErrorCode(), card.sdErrorData(),
				capacity_kib, free_kib, card.fatType(), sector_size, cluster_size);
			display->setFont(TINY_FONT);
			draw_string(display, file_system_info_text, 0);

			break;
		}
		case DiagnosticPage::__Count:
		default: {
			this->current_page = DiagnosticPage::Time;
			break;
		}
		}

		yield();
	} while (display->nextPage());
	return this;
}

bool DiagnosticMenu::should_refresh(uint16_t delta_millis)
{
	return dirty;
}

Menu* DiagnosticMenu::handle_button(uint8_t buttons)
{
	if (buttons != 0)
		this->dirty = true;

	if (buttons & BUTTON_LEFT)
		return this->parent;

	if (buttons & BUTTON_DOWN)
		this->current_page = this->current_page + 1;
	if (buttons & BUTTON_UP)
		this->current_page = this->current_page - 1;

	return this;
}
