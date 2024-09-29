#pragma once

enum class Platform_Message_Type {
	info,
	error
};

struct Platform {
	void log_error(const char *format, ...) const;
	void log_info(const char *format, ...) const;
};