
#include "Audio.h"
#include "Debug.h"
#include "PrintString.h"

extern SdFs card;

String getBaseName(FsFile& file)
{
    PrintString baseName;
    file.printName(&baseName);
    return baseName.getString();
}

void audioSetup()
{
    audioSource = new AudioFileSourceSdFs(card);
    // test with first found mp3
    FsFile dir = card.open("/");
    FsFile file;
    String baseName;
    do {
        file = dir.openNextFile();
        baseName = getBaseName(file);
    } while (file && (!(baseName.endsWith(F(".mp3"))) || (baseName.length() < 1)));

    audioSource->close();
    if (!audioSource->open((getBaseName(dir) + String("/") + baseName).c_str())) {
        return;
    }

    audioOutput = new AudioOutputI2S();
    audioPlayer = new AudioGeneratorMP3();
    audioPlayer->begin(audioSource, audioOutput);
    debug_print(F("Started playback."));
}

void audioLoop()
{
    if (audioPlayer && audioPlayer->isRunning()) {
        if (!audioPlayer->loop()) {
            audioPlayer->stop();
            debug_print(F("Audio track ended."));
        }
    }
}
