/**Audio playback system header*/

#pragma once

#include "AudioFileSourceSdFs.h"
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
	bool is_playing() const { return audio_player && audio_player->isRunning(); }

private:
	// Singleton instance
	static std::unique_ptr<AudioManager> instance;

	AudioFileSourceSdFs audio_source;
	AudioOutputI2S audio_output;
	std::unique_ptr<AudioGenerator> audio_player;
	Ticker timer;
};

// fake non-realtime cooperative "interrupt" invoked via yield()/delay()
void IRAM_ATTR audio_timer_interrupt();
