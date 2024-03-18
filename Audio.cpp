
#include "Audio.h"
#include "Debug.h"
#include "PrintString.h"
#include <AudioGeneratorFLAC.h>
// #include <AudioGeneratorMP3.h>
#include <AudioOutputBuffer.h>
#include <memory>

String get_base_name(FsFile& file)
{
	PrintString base_name;
	file.printName(&base_name);
	return base_name.getString();
}

std::unique_ptr<AudioManager> AudioManager::instance;

AudioManager& AudioManager::the()
{
	if (!AudioManager::instance)
		AudioManager::instance = std::make_unique<AudioManager>();

	return *AudioManager::instance.get();
}

AudioManager::AudioManager()
	: audio_source(card)
	, audio_output(0, AudioOutputI2S::EXTERNAL_I2S, 16,
		  AudioOutputI2S::APLL_DISABLE)
{
	timer.attach_ms(1, audio_timer_interrupt);
	// test with first found file
	FsFile dir = card.open("/");
	FsFile file;
	String base_name;
	do {
		file = dir.openNextFile();
		base_name = get_base_name(file);
	} while (file && (!(base_name.endsWith(F(".flac"))) || (base_name.length() < 1)));
	String file_name = get_base_name(dir) + base_name;
	play(file_name);
}

void AudioManager::play(String& file_name)
{
	if (audio_player)
		audio_player->stop();
	audio_source.close();

	if (file_name.endsWith(F(".flac"))) {
		audio_player = std::make_unique<AudioGeneratorFLAC>();
		// } else if (file_name.endsWith(F(".mp3"))) {
		// audio_player = std::make_unique<AudioGeneratorMP3>();
	}

	if (!audio_source.open(file_name.c_str())) {
		return;
	}
	audio_player->begin(&audio_source, &audio_output);
}

float AudioManager::current_position() const
{
	auto sample_rate = audio_output.sample_rate();
	auto sample_count = audio_output.sample_count();
	return static_cast<float>(sample_count) / sample_rate;
}

void AudioManager::loop()
{
	if (audio_player && audio_player->isRunning()) {
		if (!audio_player->loop()) {
			audio_player->stop();
			debug_print(F("Audio track ended."));
		}
	}
}

void audio_timer_interrupt() { AudioManager::the().loop(); }
