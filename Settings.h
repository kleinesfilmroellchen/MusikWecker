/** Defines the eeprom_settings struct which declares the setting data structure in (emulated) EEPROM. */
#pragma once

#include "EnumBits.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <ace_time/LocalTime.h>

// Exact time format for digital clock faces.
enum class TimeFormat : uint8_t {
	// 12-hour clock, no AM/PM information; 12:00 = midnight and midday
	Hours12,
	// 12-hour clock with AM/PM; 12:00 AM =  midnight, 12:00 PM = midday
	Hours12AmPm,
	// 24-hour clock; 00:00 = midnight, 12:00 = midday
	Hours24,
};

// How to show date information on the clock faces.
enum class DateFormat : uint8_t {
	None,
	// YYYY-MM-DD
	ISO8601,
	// DD.MM.YYYY
	German,
	// DD.MM.YY with superflous zeroes removed
	GermanShort,
};

// Clock face settings.
struct ClockSettings {
	/** Index into clock face functions list, determines selected clock face. This may be an issue*/
	uint16_t clock_face_index = 0;
	DateFormat date_format;
	TimeFormat time_format;
	// Whether to show seconds on the clock faces.
	bool show_seconds = true;
};

// How to repeat alarms; a bitflag class.
enum class AlarmRepetition : uint8_t {
	Monday = 1 << 0,
	Tuesday = 1 << 1,
	Wednesday = 1 << 2,
	Thursday = 1 << 3,
	Friday = 1 << 4,
	Saturday = 1 << 5,
	Sunday = 1 << 6,
};

AK_ENUM_BITWISE_OPERATORS(AlarmRepetition)

// Make an AlarmRepetition that repeats on all weekdays.
constexpr AlarmRepetition repeat_on_weekdays()
{
	return AlarmRepetition::Monday | AlarmRepetition::Tuesday | AlarmRepetition::Wednesday | AlarmRepetition::Thursday | AlarmRepetition::Friday;
}

// Make an AlarmRepetition that repeats every day.
constexpr AlarmRepetition repeat_daily()
{
	return AlarmRepetition::Monday | AlarmRepetition::Tuesday | AlarmRepetition::Wednesday | AlarmRepetition::Thursday | AlarmRepetition::Friday | AlarmRepetition::Saturday | AlarmRepetition::Sunday;
}

// Make an AlarmRepetition that repeats on all weekend days.
constexpr AlarmRepetition repeat_on_weekends()
{
	return AlarmRepetition::Saturday | AlarmRepetition::Sunday;
}

// Whether the alarm repeats at all.
constexpr bool does_repeat(AlarmRepetition const& repetition)
{
	return repetition != static_cast<AlarmRepetition>(0);
}

// Settings of a single alarm.
struct Alarm {
	// Time in the day when the alarm should trigger.
	ace_time::LocalTime alarm_time = ace_time::LocalTime::forError();
	AlarmRepetition repetition;
	// Whether the alarm will trigger. Non-repeating alarms are set to disabled the moment they trigger so they don’t trigger the next day again.
	bool is_enabled = false;
};

// Maximum number of alarms.
constexpr size_t ALARM_COUNT = 32;

/**
   The settings store all user customizable data except wifi SSIDs and passwords, which are stored and remembered by the ESP8266 OS automatically. This entire system assumes that the user has used the WiFi example to connect to their network.
*/
struct EepromSettings {
	uint32_t sleep_time = 5 * 60 * 1000;
	ClockSettings clock_settings {};
	/** Index into the timezone list, defined in zonelist.h */
	uint16_t timezone = 0;
	bool show_debug = false;
	Alarm alarms[ALARM_COUNT];
};

// macro for size of setting data
constexpr size_t SETTINGS_SIZE = sizeof(EepromSettings);
// macro for address of setting data. Because no other eeprom storage is used, 0 can be used here.
constexpr size_t SETTINGS_ADDRESS = 0;

/** Helper function that stores the settings struct in EEPROM, if it changed. */
void save_settings();
