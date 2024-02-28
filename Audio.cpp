
#include "Audio.h"

void audioSetup()
{ /*
audioSource = new AudioFileSourceSD();
// test with first found mp3
File dir = SD.open("/music");
File file;
do {
file = dir.openNextFile();
debug_print(F("Considering ")); debug_print(file.name());
} while ( file && (!(String(file.name()).endsWith(".mp3")) || (String(file.name()).length() < 1)) );
debug_print(F("Trying to play ")); debug_print(file.name());

audioSource->close();
if (audioSource->open((String(dir.name()) + String("/") + String(file.name())).c_str())) {
Serial.printf_P(PSTR("Playing '%s' from SD card...\n"), file.name());
} else {
Serial.printf_P(PSTR("Error opening '%s'\n"), file.name());
}

audioOutput = new AudioOutputI2SNoDAC();
audioPlayer = new AudioGeneratorMP3();
audioPlayer->begin(audioSource, audioOutput);
debug_print(F("Started playback."));*/
}

void audioLoop()
{ /*
if (audioPlayer->isRunning()) {
if (!audioPlayer->loop()) {
audioPlayer->stop();
debug_print(F("Audio track ended."));
}
}*/
}
