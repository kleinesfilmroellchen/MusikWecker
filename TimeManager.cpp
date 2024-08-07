#include "TimeManager.h"
#include "Globals.h"
#include "PrintString.h"
#include "zonelist.h"
#include <ESP8266WiFi.h>
#include <umm_malloc/umm_heap_select.h>

std::unique_ptr<TimeManager> TimeManager::instance;

TimeManager& TimeManager::the()
{
	HeapSelectIram iram;
	if (!TimeManager::instance)
		TimeManager::instance = std::make_unique<TimeManager>();

	return *TimeManager::instance.get();
}

TimeManager::TimeManager()
	: time_client(udp_client, TEMP_TIME_SERVER, 0, TEMP_NTP_UPDATE_INTERVAL)
	, manager(ace_time::zonedbc::kZoneRegistrySize, ace_time::zonedbc::kZoneRegistry, zoneProcessorCache)
	, main_time_zone(std::make_unique<ace_time::TimeZone>())
{
	time_client.begin();

	if (WiFi.status() == WL_CONNECTED) {
		ntp_update_occurred = time_client.update();
	}
}

String TimeManager::date_text_for_format(DateFormat format) const
{
	char date_text[11] {};
	auto the_current_time = current_time();
	switch (format) {
	case DateFormat::None:
		break;
	case DateFormat::ISO8601:
		snprintf_P(date_text, sizeof(date_text), PSTR("%04u-%02u-%02u"),
			the_current_time.year(), the_current_time.month(), the_current_time.day());
		break;
	case DateFormat::German:
		snprintf_P(date_text, sizeof(date_text), PSTR("%02u.%02u.%4u"), the_current_time.day(), the_current_time.month(),
			the_current_time.year());
		break;
	case DateFormat::GermanShort: {
		const uint8_t year_2digit = the_current_time.year() % 100;
		snprintf_P(date_text, sizeof(date_text), PSTR("%02u.%02u.%02u"), the_current_time.day(), the_current_time.month(),
			year_2digit);
		break;
	}
	}
	return { date_text };
}

String TimeManager::zone_name() const
{
	PrintString zone_name_text;
	main_time_zone->printTo(zone_name_text);
	return zone_name_text.getString();
}

void TimeManager::set_zone(uint16_t zone_index)
{
	String tzname = FPSTR(tzlist[zone_index]);
	*main_time_zone = manager.createForZoneName(tzname.c_str());
	eeprom_settings.timezone = zone_index;
	save_settings();
}

void TimeManager::update_if_needed()
{
	// sync own time with ntp server, if wifi is available.
	if (WiFi.status() == WL_CONNECTED) {
		ntp_update_occurred = time_client.update();
	} else {
		ntp_update_occurred = false;
	}

	if (ntp_update_occurred)
		time_of_last_ntp_update = millis();
}
