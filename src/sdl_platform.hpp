#pragma once

#include <SDL2/SDL.h>

#include "platform.hpp"

struct SDL_Platform : Platform {
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
};