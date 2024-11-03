#pragma once

template <typename T>
struct Size {
	T width, height;

	bool operator ==(const Size<T> &other) const {
		return this->width == other.width && this->height == other.height;
	}
};