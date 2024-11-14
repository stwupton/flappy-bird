#pragma once

#include <string>

struct Platform {
	virtual void log_error(const char *format, ...) const = 0;
	virtual void log_info(const char *format, ...) const = 0;
	virtual void save_high_score(int score) const = 0;
	virtual int get_high_score() const = 0;
	virtual const std::string get_asset_path() const = 0;
};