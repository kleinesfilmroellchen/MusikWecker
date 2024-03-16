# Developer Guide

Use the Arduino IDE or CLI. There's VSCode integration too!

clangd or the Microsoft C/C++ language server should work well.

Formatting with clang-format is necessary for contributions. The config is in the standard .clang-format, MusikWecker uses minimally modified WebKit style.

## TODOs because I forget

Some of the things I still need to implement

- The entire alarm system
	- Creating and deleting alarms
	- Alarm saving and restoring (EEPROM is fixed size; how to implement dynamic alarm clock count?)
	- Alarm settings (which are available?)
	- Executing alarms
	- Correct wakeup before an alarm runs (at least 10s to allow WiFi reconnect)
- Software volume control
- Power saving shenanigans
	- Core clock-down while we're not playing music. 160MHz is really only needed for the high SPI+I2S+audio decode workload.
	- Use an actually preempting hardware timer for audio playback. timer1 _should_ work, but we need to reach into Espressif C APIs for this.
	- Figure out why light sleep is so crashy
	- Periodically do fake wakeups from light sleep every 10s or so to trick "smart" battery banks into not shutting off power
- OTA
- Play music when selected in file view
	- Play entire folder
	- a fucking MP3 player (the hardware is here anyways)
- All the settings:
	- Disable/enable date display
	- Configure clock format for digital clocks (12h/24h)
	- Configure date format
	- Configure NTP update interval
	- Configure NTP server
- Put together a BOM for making a single PCB for this
- Design a single PCB for this
- Manufacture a single PCB for this
- World domination