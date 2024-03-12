/**Audio playback system header*/

#pragma once

#include "AudioFileSourceSdFs.h"
#include "defs.h"
#include <AudioGenerator.h>
#include <AudioOutput.h>
#include <AudioOutputI2S.h>
#include <memory>

extern std::unique_ptr<AudioFileSourceSdFs> audioSource;
extern std::unique_ptr<AudioOutputI2S> audioOutput;
extern std::unique_ptr<AudioGenerator> audioPlayer;

void audioSetup();
/** Called during audio playback; handles playing the audio.*/
void audioLoop();
