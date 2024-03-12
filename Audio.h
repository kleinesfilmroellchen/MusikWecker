/**Audio playback system header*/

#pragma once

#include "AudioFileSourceSdFs.h"
#include "defs.h"
#include <AudioGenerator.h>
#include <AudioOutput.h>
#include <AudioOutputI2S.h>
#include <Ticker.h>
#include <memory>

class AudioManager {
public:
    static AudioManager& the();
    AudioManager();

    void loop();
    bool isPlaying() const { return audioPlayer && audioPlayer->isRunning(); }

private:
    // Singleton instance
    static std::unique_ptr<AudioManager> instance;

    AudioFileSourceSdFs audioSource;
    AudioOutputI2S audioOutput;
    std::unique_ptr<AudioGenerator> audioPlayer;
    Ticker timer;
};

void audioTimerInterrupt();
