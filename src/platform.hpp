#pragma once

#include <string>

struct Platform_File {
	unsigned int content_size;
	char *contents;
};

struct Platform {
	virtual void log_error(const char *format, ...) const = 0;
	virtual void log_info(const char *format, ...) const = 0;
	virtual void save_high_score(int score) const = 0;
	virtual int get_high_score() const = 0;
	virtual void load_file(const char *path, Platform_File **file) const = 0;
	virtual void close_file(Platform_File **file) const = 0;
	virtual const std::string get_asset_path(const char *file_path) const = 0;
};