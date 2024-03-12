
#include "Audio.h"
#include "Debug.h"
#include "PrintString.h"
#include <AudioGeneratorFLAC.h>
#include <AudioOutputBuffer.h>
#include <memory>

extern SdFs card;

String getBaseName(FsFile& file)
{
    PrintString baseName;
    file.printName(&baseName);
    return baseName.getString();
}

std::unique_ptr<AudioManager> AudioManager::instance;

AudioManager& AudioManager::the()
{
    if (!AudioManager::instance)
        AudioManager::instance = std::make_unique<AudioManager>();

    return *AudioManager::instance.get();
}

AudioManager::AudioManager()
    : audioSource(card)
    , audioOutput(0, AudioOutputI2S::EXTERNAL_I2S, 16,
          AudioOutputI2S::APLL_DISABLE)
{
    timer.attach_ms(1, audioTimerInterrupt);

    // test with first found file
    FsFile dir = card.open("/");
    FsFile file;
    String baseName;
    do {
        file = dir.openNextFile();
        baseName = getBaseName(file);
    } while (file && (!(baseName.endsWith(F(".flac"))) || (baseName.length() < 1)));

    audioSource.close();
    String fileName = getBaseName(dir) + baseName;
    debug_print(fileName);
    if (!audioSource.open(fileName.c_str())) {
        return;
    }

    audioPlayer = std::make_unique<AudioGeneratorFLAC>();
    audioPlayer->begin(&audioSource, &audioOutput);
    debug_print(F("Started playback."));
}

void AudioManager::loop()
{
    if (audioPlayer && audioPlayer->isRunning()) {
        if (!audioPlayer->loop()) {
            audioPlayer->stop();
            debug_print(F("Audio track ended."));
        }
    }
}

void audioTimerInterrupt() { AudioManager::the().loop(); }
