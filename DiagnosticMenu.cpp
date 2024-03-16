#include "DiagnosticMenu.h"
#include "Audio.h"
#include "DisplayUtils.h"
#include "Globals.h"
#include "NTPClient.h"
#include "PrintString.h"
#include "strings.h"
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
		switch (this->currentPage) {
		case DiagnosticPage::Time: {
			this->dirty = true;

			PrintString zoneName;
			main_time_zone->printTo(zoneName);

			char timeInfo[128];
			snprintf_P(
				timeInfo, sizeof(timeInfo),
				PSTR("unix %ds\ntz %s offset %05ds\nserver %s\nlast sync %d (synced "
					 "%d)"),
				time_client.getEpochTime(), zoneName.getString().c_str(),
				main_time_zone->getOffsetDateTime(time_client.getEpochTime()).timeOffset(),
				TEMP_TIME_SERVER, millis() - timeOfLastNTP, time_client.isTimeSet());
			display->setFont(TINY_FONT);
			draw_string(display, timeInfo, 0);
			break;
		}

		case DiagnosticPage::FileSystem: {
			this->dirty = true;

			auto capacityKiB = static_cast<uint64_t>(card.vol()->clusterCount() / 1024) * card.vol()->bytesPerCluster();
			auto freeKiB = static_cast<uint64_t>(card.vol()->freeClusterCount() / 1024) * card.vol()->bytesPerCluster();
			auto sectorSize = card.vol()->bytesPerCluster() / card.vol()->sectorsPerCluster();
			auto clusterSize = card.vol()->bytesPerCluster();
			auto sdNumericType = card.card()->type() % 4;
			String sdTypeName = FPSTR(sd_types_array[sdNumericType]);

			char fsInfo[256] {};
			snprintf_P(fsInfo, sizeof(fsInfo),
				PSTR("sd type %s\nerror %d payload %d\n%lldKi cap %lldKi "
					 "free\nFAT%d: %d secsz %d clusz"),
				sdTypeName, card.sdErrorCode(), card.sdErrorData(),
				capacityKiB, freeKiB, card.fatType(), sectorSize, clusterSize);
			display->setFont(TINY_FONT);
			draw_string(display, fsInfo, 0);

			break;
		}
		case DiagnosticPage::__Count: {
			this->currentPage = DiagnosticPage::Time;
			break;
		}
		}

		yield();
	} while (display->nextPage());
	return this;
}

bool DiagnosticMenu::should_refresh(uint16_t delta_millis)
{
	if (ntp_update_occurred)
		this->timeOfLastNTP = millis();

	return dirty;
}

Menu* DiagnosticMenu::handle_button(uint8_t buttons)
{
	if (buttons != 0)
		this->dirty = true;

	if (buttons & B_LEFT)
		return this->parent;

	if (buttons & B_DOWN)
		this->currentPage = this->currentPage + 1;
	if (buttons & B_UP)
		this->currentPage = this->currentPage - 1;

	return this;
}
