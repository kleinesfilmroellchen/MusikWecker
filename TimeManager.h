/**Real-time clock, NTP and time zone management*/

#pragma once

#include "Definitions.h"
#include "Settings.h"
#include <AceTime.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ace_time/ZonedDateTime.h>
#include <memory>

class TimeManager {
public:
	static TimeManager& the();
	TimeManager();

	uint64_t epoch_time() const { return time_client.getEpochTime(); }
	bool has_network_time() const { return time_client.isTimeSet(); }
	bool has_ntp_update_occurred() const { return ntp_update_occurred; }
	uint32_t time_since_ntp_update() const { return millis() - time_of_last_ntp_update; }
	ace_time::ZonedDateTime current_time() const
	{
		return ace_time::ZonedDateTime::forUnixSeconds64(time_client.getEpochTime(), *main_time_zone);
	}
	void update_if_needed();
	void force_update() { time_client.forceUpdate(); }

	String zone_name() const;
	void set_zone(uint16_t zone_index);

	String date_text_for_format(DateFormat format) const;

private:
	// Singleton instance
	static std::unique_ptr<TimeManager> instance;

	WiFiUDP udp_client {};
	NTPClient time_client;
	ace_time::CompleteZoneProcessorCache<TIME_ZONE_CACHE_SIZE> zoneProcessorCache {};
	std::unique_ptr<ace_time::TimeZone> main_time_zone;
	ace_time::CompleteZoneManager manager;

	uint32_t time_of_last_ntp_update {};
	bool ntp_update_occurred { false };
};
