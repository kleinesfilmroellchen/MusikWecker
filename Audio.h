/**Audio playback system header*/

#pragma once

#include "defs.h"

// esp audio library
#include <AudioFileSource.h>
#include <AudioFileSourceSD.h>
#include <AudioGenerator.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorMP3a.h>
#include <AudioGeneratorRTTTL.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutput.h>
#include <AudioOutputBuffer.h>
//#include <AudioOutputFilterDecimate.h>
#include <AudioOutputI2SNoDAC.h>

static AudioFileSourceSD* audioSource = nullptr;
static AudioOutputI2SNoDAC* audioOutput = nullptr;
static AudioGenerator* audioPlayer = nullptr;

void audioSetup();
/** Called during audio playback; handles playing the audio.*/
void audioLoop();
