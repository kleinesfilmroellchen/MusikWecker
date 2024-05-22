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
static const char* hour_names[] PROGMEM {
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

static const char main_menu_0[] PROGMEM = "Uhrdesign";
static const char main_menu_1[] PROGMEM = "Wecker";
static const char main_menu_2[] PROGMEM = "Dateiverwaltung";
static const char main_menu_3[] PROGMEM = "Einstellungen";
static const char main_menu_4[] PROGMEM = "Diagnostik";
static const char* main_menu_array[] PROGMEM = { main_menu_0, main_menu_1, main_menu_2, main_menu_3, main_menu_4 };
static const Span<char const*> main_menu { main_menu_array };

static const char design_menu_0[] PROGMEM = "Digital";
static const char design_menu_1[] PROGMEM = "Digital + Sekunden";
static const char design_menu_2[] PROGMEM = "Analog (minimalistisch)";
static const char design_menu_3[] PROGMEM = "Analog (modern)";
static const char design_menu_4[] PROGMEM = "Rotierende Segmente";
static const char design_menu_5[] PROGMEM = "Binär";
static const char design_menu_6[] PROGMEM = "Binär (Tagsekunden)";
static const char* design_menu_array[] PROGMEM = { design_menu_0, design_menu_1, design_menu_2, design_menu_3, design_menu_4, design_menu_5, design_menu_6 };
static const Span<char const*> design_menu { design_menu_array };

static const char waketone_menu_0[] PROGMEM = "Zufällig (Reihe)";
static const char waketone_menu_1[] PROGMEM = "Zufällig (echt)";
static const char waketone_menu_2[] PROGMEM = "Festgelegt";
static const char* waketone_menu_array[] PROGMEM = { waketone_menu_0, waketone_menu_1, waketone_menu_2 };
static const Span<char const*> waketone_menu { waketone_menu_array };

static const char file_menu_0[] PROGMEM = "Dateiansicht";
static const char file_menu_1[] PROGMEM = "Verschieben";
static const char file_menu_2[] PROGMEM = "Löschen";
static const char* file_menu_array[] PROGMEM = { file_menu_0, file_menu_1, file_menu_2 };
static const Span<char const*> file_menu { file_menu_array };

static const char settings_menu_0[] PROGMEM = "Auto-Abschaltung";
static const char settings_menu_1[] PROGMEM = "Zeitzone";
static const char settings_menu_2[] PROGMEM = "Datumsanzeige";
static const char settings_menu_3[] PROGMEM = "Uhrzeitformat";
static const char settings_menu_4[] PROGMEM = "Debugging";
static const char* settings_menu_array[] PROGMEM = { settings_menu_0, settings_menu_1, settings_menu_2, settings_menu_3, settings_menu_4 };
static const Span<char const*> settings_menu { settings_menu_array };

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
