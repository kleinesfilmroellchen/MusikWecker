
#include "Audio.h"
#include "Debug.h"
#include "PrintString.h"
#include <AudioGeneratorFLAC.h>
#include <AudioOutputBuffer.h>
#include <memory>

extern SdFs card;

std::unique_ptr<AudioFileSourceSdFs> audioSource = nullptr;
std::unique_ptr<AudioOutputI2S> audioOutput = nullptr;
std::unique_ptr<AudioOutputBuffer> audioBuffer = nullptr;
std::unique_ptr<AudioGenerator> audioPlayer = nullptr;

String getBaseName(FsFile& file)
{
    PrintString baseName;
    file.printName(&baseName);
    return baseName.getString();
}

void audioSetup()
{
    audioSource = std::make_unique<AudioFileSourceSdFs>(card);
    // test with first found file
    FsFile dir = card.open("/");
    FsFile file;
    String baseName;
    do {
        file = dir.openNextFile();
        baseName = getBaseName(file);
    } while (file && (!(baseName.endsWith(F(".flac"))) || (baseName.length() < 1)));

    audioSource->close();
    String fileName = getBaseName(dir) + baseName;
    debug_print(fileName);
    if (!audioSource->open(fileName.c_str())) {
        return;
    }

    audioOutput = std::make_unique<AudioOutputI2S>(0, AudioOutputI2S::EXTERNAL_I2S, 16, AudioOutputI2S::APLL_DISABLE);
    // audioBuffer = std::make_unique<AudioOutputBuffer>(512, audioOutput.get());
    audioPlayer = std::make_unique<AudioGeneratorFLAC>();
    audioPlayer->begin(audioSource.get(), audioOutput.get());
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
