/** String constants, to be stored in PROGMEM (flash) */

#pragma once

#include "Span.h"
#include <Arduino.h>

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
static const char settings_menu_2[] PROGMEM = "Debugging";
static const char* settings_menu_array[] PROGMEM = { settings_menu_0, settings_menu_1, settings_menu_2 };
static const Span<char const*> settings_menu { settings_menu_array };

static const char* yes PROGMEM = "Ja";
static const char* no PROGMEM = "Nein";
static const std::array<char const*, 2> yes_no_array = { no, yes };
static const Span<char const*> yes_no_menu { yes_no_array };

static const char* auto_disable_menu_0 PROGMEM = "Deaktiviert";
static const char* auto_disable_menu_1 PROGMEM = "10 Sekunden";
static const char* auto_disable_menu_2 PROGMEM = "5 Minuten";
static const char* auto_disable_menu_3 PROGMEM = "10 Minuten";
static const char* auto_disable_menu_4 PROGMEM = "30 Minuten";
static const std::array<char const*, 5> auto_disable_array = { auto_disable_menu_0, auto_disable_menu_1, auto_disable_menu_2, auto_disable_menu_3, auto_disable_menu_4 };
static const Span<char const*> auto_disable_menu { auto_disable_array };

static const char* sd_types_0 PROGMEM = "invalid";
static const char* sd_types_1 PROGMEM = "SD1";
static const char* sd_types_2 PROGMEM = "SD2";
static const char* sd_types_3 PROGMEM = "SDHC/SDXC";
static const std::array<char const*, 4> sd_types_array = { sd_types_0, sd_types_1, sd_types_2, sd_types_3 };

static const char* debugging_label PROGMEM = "Firmware-Debugging\naktivieren";
static const char* auto_disable_label PROGMEM = "Bildschirm abschalten\nbei Inaktivität";
static const char* confirm_delete_label PROGMEM = "Wirklich löschen?";
static const char* confirm_move_label PROGMEM = "Wirklich hierher\nverschieben?";
