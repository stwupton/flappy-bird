#pragma once

#include <string>

#include <SDL2/SDL.h>

#include "audio_player.hpp"

struct Audio {
	SDL_AudioSpec spec;
	Uint8 *buffer;
	Uint32 length;
	Uint32 current_position;
};

struct SDL_Audio_Player : Audio_Player {
	Platform &platform;

	SDL_Audio_Player(Platform &platform) : platform{platform} {}

private:
	Audio flap_audio;
	Audio score_audio;
	Audio hit_audio;

public:
	bool init() {
		const std::string flap_audio_path = this->platform.get_asset_path(Asset::get_audio(Asset::Audio_ID::flap));
		bool success = this->load(flap_audio_path, &this->flap_audio);
		if (!success) {
			return false;
		}

		const std::string score_audio_path = this->platform.get_asset_path(Asset::get_audio(Asset::Audio_ID::score));
		success = this->load(score_audio_path, &this->score_audio);
		if (!success) {
			return false;
		}

		const std::string hit_audio_path = this->platform.get_asset_path(Asset::get_audio(Asset::Audio_ID::hit));
		success = this->load(hit_audio_path, &this->hit_audio);
		if (!success) {
			return false;
		}

		// Use the flap audio spec as the desired audio spec, I don't thing it 
		// matters too much what the spec is here.
		SDL_AudioSpec desired = flap_audio.spec;
		desired.userdata = this;
		desired.callback = audio_callback;

		// Only works if `obtained` is NULL. Not sure why.
		if (SDL_OpenAudio(&desired, NULL) < 0) {
			SDL_Log(SDL_GetError());
			return false;
		}

		// Play all audio
		SDL_PauseAudio(0);

		return true;
	}

	void flap() override {
		this->play(&this->flap_audio);
	}

	void score() override {
		this->play(&this->score_audio);
	}

	void hit() override {
		this->play(&this->hit_audio);
	}

private:
	bool load(std::string path, Audio *audio) {
		SDL_AudioSpec *spec = SDL_LoadWAV(
			path.c_str(), 
			&audio->spec, 
			&audio->buffer, 
			&audio->length
		);

		if (spec == nullptr) {
			SDL_Log(SDL_GetError());
			return false;
		}

		this->stop(audio);
		return true;
	}

	void play(Audio *audio) {
		audio->current_position = 0;
	}

	void stop(Audio *audio) {
		audio->current_position = audio->length;
	}

	static void mix_audio(Uint8 *stream, int stream_length, Audio *audio) {
		const Uint8 *buffer_start_position = audio->buffer + audio->current_position;
		const Uint32 remaining_sound_length = audio->length - audio->current_position;
		const Uint32 length = stream_length > remaining_sound_length ? remaining_sound_length : stream_length;
		SDL_MixAudio(stream, buffer_start_position, length, SDL_MIX_MAXVOLUME);
		audio->current_position += length;
	}

	static void audio_callback(void *data, Uint8 *stream, int stream_length) {
		SDL_AudioSpec device_spec;
		SDL_GetAudioDeviceSpec(NULL, 0, &device_spec);
		SDL_memset(stream, device_spec.silence, stream_length);

		if (stream_length == 0) {
			return;
		}
	
		SDL_Audio_Player *player = (SDL_Audio_Player*)data;
		mix_audio(stream, stream_length, &player->flap_audio);
		mix_audio(stream, stream_length, &player->score_audio);
		mix_audio(stream, stream_length, &player->hit_audio);
	}
};