
#include "Audio.h"
#include "Debug.h"
#include "PrintString.h"
#include "user_interface.h"
#include <AudioGeneratorFLAC.h>
#include <umm_malloc/umm_heap_select.h>
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
	HeapSelectIram iram;

	if (!AudioManager::instance)
		AudioManager::instance = std::make_unique<AudioManager>();

	return *AudioManager::instance.get();
}

AudioManager::AudioManager()
	: audio_source(card)
	, audio_output(0, AudioOutputI2S::EXTERNAL_I2S, 8,
		  AudioOutputI2S::APLL_DISABLE)
{
	timer.attach_ms(1, audio_timer_interrupt);
}

void metadata_callback(void*, char const* key, bool is_unicode, char const* value)
{
	// TODO: Provide the metadata back to the AudioManager so UI can read it out.
	// PrintString output;
	// output.printf_P(PSTR("Audio: Metadata: %s = %s\n"), key, value);
	// debug_print(output.getString());
}

void error_callback(void*, int code, char const* string)
{
	// PrintString output;
	// output.printf_P(PSTR("Audio: Status code %d: %s\n"), code, string);
	// debug_print(output.getString());
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
	audio_player->RegisterMetadataCB(&metadata_callback, nullptr);
	audio_player->RegisterStatusCB(&error_callback, nullptr);
	audio_player->begin(&audio_source, &audio_output);
	debug_print(F("Audio: Starting playback"));
}

float AudioManager::current_position() const
{
	auto sample_rate = audio_output.sample_rate();
	auto sample_count = audio_output.sample_count();
	return static_cast<float>(static_cast<double>(sample_count) / sample_rate);
}

enum class Frequency : bool {
	Mhz160,
	Mhz80,
};

void set_frequency(Frequency frequency)
{
	auto target_freq = frequency == Frequency::Mhz160 ? SYS_CPU_160MHZ : SYS_CPU_80MHZ;
	if (system_get_cpu_freq() != target_freq) {
		system_update_cpu_freq(target_freq);

		if (frequency == Frequency::Mhz160)
			debug_print(F("CPU: Clocking to 160 MHz"));
		else
			debug_print(F("CPU: Clocking to 80 MHz"));
	}
}

void AudioManager::loop()
{
	if (audio_player && audio_player->isRunning()) {
		set_frequency(Frequency::Mhz160);
		if (!audio_player->loop()) {
			audio_player->stop();
			debug_print(F("Audio: Track ended."));
		}
	} else {
		set_frequency(Frequency::Mhz80);
	}
}

void audio_timer_interrupt() { AudioManager::the().loop(); }
