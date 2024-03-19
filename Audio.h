/**Audio playback system header*/

#pragma once

#include "AudioFileSourceSdFs.h"
#include <AudioGenerator.h>
#include <AudioOutput.h>
#include <AudioOutputI2S.h>
#include <Ticker.h>
#include <memory>

template <typename UnderlyingOutput>
class SampleCounterOutput : public UnderlyingOutput {
public:
	using UnderlyingOutput::UnderlyingOutput;
	virtual ~SampleCounterOutput() = default;

	int sample_rate() const { return this->hertz; }
	size_t sample_count() const { return the_sample_count; }
	virtual bool ConsumeSample(int16_t sample[2]) override
	{
		the_sample_count++;
		return UnderlyingOutput::ConsumeSample(sample);
	}
	virtual bool stop() override
	{
		the_sample_count = 0;
		return UnderlyingOutput::stop();
	}

private:
	size_t the_sample_count { 0 };
};

class AudioManager {
public:
	static AudioManager& the();
	AudioManager();

	void loop();
	bool is_playing() const { return audio_player && audio_player->isRunning(); }

	void play(String& file_name);

	float current_position() const;
	size_t played_sample_count() const { return audio_output.sample_count();}
	size_t sample_rate() const { return audio_output.sample_rate();}

private:
	// Singleton instance
	static std::unique_ptr<AudioManager> instance;

	AudioFileSourceSdFs audio_source;
	SampleCounterOutput<AudioOutputI2S> audio_output;
	std::unique_ptr<AudioGenerator> audio_player;
	Ticker timer;
};

// fake non-realtime cooperative "interrupt" invoked via yield()/delay()
void IRAM_ATTR audio_timer_interrupt();
