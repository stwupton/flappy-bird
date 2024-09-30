#pragma once

struct Platform {
	virtual void log_error(const char *format, ...) const = 0;
	virtual void log_info(const char *format, ...) const = 0;
};