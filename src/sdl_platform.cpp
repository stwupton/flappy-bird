#include <SDL2/SDL.h>

#include "platform.hpp"

void Platform::log_error(const char *format, ...) const {
	va_list args;
	va_start(args, format);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_ERROR, format, args);
	va_end(args);
}

void Platform::log_info(const char *format, ...) const {
	va_list args;
	va_start(args, format);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LogPriority::SDL_LOG_PRIORITY_INFO, format, args);
	va_end(args);
}