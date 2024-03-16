# MusikWecker

MusikWecker ("music alarm clock") is an ESP8266-based alarm clock that plays music instead of horrible beeps.

Features:
- Music playback from SD card; various common formats like AAC, MP3, FLAC, WAV etc. supported
- Multiple, customizable alarms with various music playback modes
- Time synchronization via NTP
- Accurate local time and time zone support
- Many aesthetically appealing clock faces
- Low power consumption thanks to ESP8266 light sleep

## Development

Use a recent Arduino IDE or CLI, and make sure all libraries are up-to-date. (Library definition in a sketch config will follow shortly.)

Generate the auto-generated files by running `python .`. (Python >= 3.9, see requirements.txt) This is necessary when you change the generator scripts or when modifying or adding graphics. The compiler will only complain about missing headers the first time you forget this.

