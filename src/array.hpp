#pragma once

#include <cassert>

template<typename T, size_t Size>
struct Array {
	Array() {}

	size_t length = 0;
	T data[Size];

	T &operator [](size_t index) {
		assert(index >= 0 && index < Size);
		return this->data[index];
	}

	T operator [](size_t index) const {
		assert(index >= 0 && index < Size);
		return this->data[index];
	}

	T *begin() {
		return this->data;
	}

	const T *begin() const {
		return this->data;
	}

	void clear() {
		this->length = 0;
	}

	T *end() {
		return &this->data[this->length];
	}

	const T *end() const {
		return &this->data[this->length];
	}

	T pop() {
		return this->data[--this->length];
	}

	T &push(T value) {
		assert(this->length != Size);
		return this->data[this->length++] = value;
	}
};