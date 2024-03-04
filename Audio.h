/**Audio playback system header*/

#pragma once

#include "AudioFileSourceSdFs.h"
#include "defs.h"
#include <AudioFileSource.h>
#include <AudioGenerator.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorMP3a.h>
#include <AudioGeneratorRTTTL.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutput.h>
#include <AudioOutputBuffer.h>
#include <AudioOutputI2S.h>

static AudioFileSourceSdFs* audioSource = nullptr;
static AudioOutputI2S* audioOutput = nullptr;
static AudioGenerator* audioPlayer = nullptr;

void audioSetup();
/** Called during audio playback; handles playing the audio.*/
void audioLoop();
