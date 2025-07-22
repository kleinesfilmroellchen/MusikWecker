/** String constants, to be stored in PROGMEM (flash) */

#pragma once

#include "Span.h"
#include <Arduino.h>

static const char hour_name_12[] PROGMEM = "12";
static const char hour_name_1[] PROGMEM = "1";
static const char hour_name_2[] PROGMEM = "2";
static const char hour_name_3[] PROGMEM = "3";
static const char hour_name_4[] PROGMEM = "4";
static const char hour_name_5[] PROGMEM = "5";
static const char hour_name_6[] PROGMEM = "6";
static const char hour_name_7[] PROGMEM = "7";
static const char hour_name_8[] PROGMEM = "8";
static const char hour_name_9[] PROGMEM = "9";
static const char hour_name_10[] PROGMEM = "10";
static const char hour_name_11[] PROGMEM = "11";
static const char* hour_names_modern[] PROGMEM {
	hour_name_12,
	hour_name_1,
	hour_name_2,
	hour_name_3,
	hour_name_4,
	hour_name_5,
	hour_name_6,
	hour_name_7,
	hour_name_8,
	hour_name_9,
	hour_name_10,
	hour_name_11,
};

static const char hour_name_xii[] PROGMEM = "XII";
static const char hour_name_i[] PROGMEM = "I";
static const char hour_name_ii[] PROGMEM = "II";
static const char hour_name_iii[] PROGMEM = "III";
static const char hour_name_iv[] PROGMEM = "IV";
static const char hour_name_v[] PROGMEM = "V";
static const char hour_name_vi[] PROGMEM = "VI";
static const char hour_name_vii[] PROGMEM = "VII";
static const char hour_name_viii[] PROGMEM = "VIII";
static const char hour_name_ix[] PROGMEM = "IX";
static const char hour_name_x[] PROGMEM = "X";
static const char hour_name_xi[] PROGMEM = "XI";
static const char* hour_names_retro[] PROGMEM {
	hour_name_xii,
	hour_name_i,
	hour_name_ii,
	hour_name_iii,
	hour_name_iv,
	hour_name_v,
	hour_name_vi,
	hour_name_vii,
	hour_name_viii,
	hour_name_ix,
	hour_name_x,
	hour_name_xi,
};

static const char main_menu_alarms[] PROGMEM = "Wecker";
static const char main_menu_files[] PROGMEM = "Dateiverwaltung";
static const char main_menu_settings[] PROGMEM = "Einstellungen";
static const char main_menu_diagnostics[] PROGMEM = "Diagnostik";
static const char main_menu_video[] PROGMEM = "Video";

static const char design_menu_digital[] PROGMEM = "Digital";
static const char design_menu_analog[] PROGMEM = "Analog (minimalistisch)";
static const char design_menu_analog_retro[] PROGMEM = "Analog (retro)";
static const char design_menu_analog_modern[] PROGMEM = "Analog (modern)";
static const char design_menu_rotating_segments[] PROGMEM = "Rotierende Segmente";
static const char design_menu_binary[] PROGMEM = "Binär";
static const char design_menu_binary_day[] PROGMEM = "Binär (Tagsekunden)";
static const char* design_menu_array[] PROGMEM = { design_menu_digital, design_menu_analog, design_menu_analog_retro, design_menu_analog_modern, design_menu_rotating_segments, design_menu_binary, design_menu_binary_day };
static const Span<char const*> design_menu { design_menu_array };

static const char waketone_menu_0[] PROGMEM = "Zufällig (Reihe)";
static const char waketone_menu_1[] PROGMEM = "Zufällig (echt)";
static const char waketone_menu_2[] PROGMEM = "Festgelegt";
static const char* waketone_menu_array[] PROGMEM = { waketone_menu_0, waketone_menu_1, waketone_menu_2 };
static const Span<char const*> waketone_menu { waketone_menu_array };

static const char file_menu_view[] PROGMEM = "Dateiansicht";
static const char file_menu_move[] PROGMEM = "Verschieben";
static const char file_menu_delete[] PROGMEM = "Löschen";

static const char settings_menu_auto_disable[] PROGMEM = "Auto-Abschaltung";
static const char settings_menu_timezone[] PROGMEM = "Zeitzone";
static const char settings_menu_clock_design[] PROGMEM = "Uhrdesign";
static const char settings_menu_date_format[] PROGMEM = "Datumsanzeige";
static const char settings_menu_time_format[] PROGMEM = "Uhrzeitformat";
static const char settings_menu_debugging[] PROGMEM = "Debugging";

static const char* yes PROGMEM = "Ja";
static const char* no PROGMEM = "Nein";
static const std::array<char const*, 2> yes_no_array = { no, yes };
static const Span<char const*> yes_no_menu { yes_no_array };

static const char* deactivated_text PROGMEM = "Deaktiviert";

static const char* auto_disable_menu_0 PROGMEM = deactivated_text;
static const char* auto_disable_menu_1 PROGMEM = "10 Sekunden";
static const char* auto_disable_menu_2 PROGMEM = "5 Minuten";
static const char* auto_disable_menu_3 PROGMEM = "10 Minuten";
static const char* auto_disable_menu_4 PROGMEM = "30 Minuten";
static const std::array<char const*, 5> auto_disable_array = { auto_disable_menu_0, auto_disable_menu_1, auto_disable_menu_2, auto_disable_menu_3, auto_disable_menu_4 };
static const Span<char const*> auto_disable_menu { auto_disable_array };

static const char* date_settings_menu_0 PROGMEM = deactivated_text;
static const char* date_settings_menu_1 PROGMEM = "ISO 8601 (JJJJ-MM-TT)";
static const char* date_settings_menu_2 PROGMEM = "Deutsch (TT.MM.JJJJ)";
static const char* date_settings_menu_3 PROGMEM = "Deutsch kurz (TT.MM.JJ)";
static const std::array<char const*, 4> date_settings_array = { date_settings_menu_0, date_settings_menu_1, date_settings_menu_2, date_settings_menu_3 };
static const Span<char const*> date_settings_menu { date_settings_array };

static const char* sd_types_0 PROGMEM = "invalid";
static const char* sd_types_1 PROGMEM = "SD1";
static const char* sd_types_2 PROGMEM = "SD2";
static const char* sd_types_3 PROGMEM = "SDHC/SDXC";
static const std::array<char const*, 4> sd_types_array = { sd_types_0, sd_types_1, sd_types_2, sd_types_3 };

static const char* debugging_label PROGMEM = "Firmware-Debugging\naktivieren";
static const char* auto_disable_label PROGMEM = "Bildschirm abschalten\nbei Inaktivität";
static const char* confirm_delete_label PROGMEM = "Wirklich löschen?";
static const char* confirm_move_label PROGMEM = "Wirklich hierher\nverschieben?";
static const char* date_settings_label PROGMEM = "Datumsanzeige auf dem\nUhrenbildschirm";

static char const* twelve_hour_format PROGMEM = "12 Stunden";
static char const* twelve_hour_am_pm_format PROGMEM = "12 Std. (AM/PM)";
static char const* twenty_four_hour_format PROGMEM = "24 Stunden";
static char const* show_seconds PROGMEM = "Sekunden anzeigen";
static const std::array<char const*, 4> time_formats = { twelve_hour_format, twelve_hour_am_pm_format, twenty_four_hour_format, show_seconds };

static char const* am PROGMEM = "AM";
static char const* pm PROGMEM = "PM";
