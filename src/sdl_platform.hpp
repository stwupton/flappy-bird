#pragma once

#include <cstdlib>
#include <string>

#include <SDL2/SDL.h>

#include "platform.hpp"

struct SDL_Platform : Platform {
private:
	SDL_RWops *save_file;

public:
	SDL_Platform() {
		std::string user_path = SDL_GetPrefPath("Shy Zone", "Flappy Bird");
		std::string save_file_path = user_path + "save";
		this->save_file = SDL_RWFromFile(save_file_path.c_str(), "r+");
		if (this->save_file == nullptr) {
			this->save_file = SDL_RWFromFile(save_file_path.c_str(), "w+");
		}
	}

	~SDL_Platform() {
		SDL_RWclose(this->save_file);
	}

	void log_error(const char *format, ...) const override {
		va_list args;
		va_start(args, format);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_ERROR, format, args);
		va_end(args);
	}

	void log_info(const char *format, ...) const override {
		va_list args;
		va_start(args, format);
		SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_INFO, format, args);
		va_end(args);
	}

	void save_high_score(int score) const override {
		char buffer[128] = {};
		_itoa_s(score, buffer, 10);
		SDL_RWseek(this->save_file, 0, RW_SEEK_SET);
		SDL_RWwrite(this->save_file, buffer, sizeof(buffer), 1);
	}

	int get_high_score() const override {
		const Sint64 file_size = SDL_RWsize(this->save_file);
		char *contents = (char *)malloc(sizeof(char) * (file_size + 1));
		SDL_RWread(this->save_file, contents, sizeof(char), file_size);
		return _atoi64(contents);
	}
};