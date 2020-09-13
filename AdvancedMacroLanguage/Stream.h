#pragma once
#include <stdio.h>
enum class streamMode {
	input, output
};
//RAII for FILE*
struct Stream {
	FILE* str;
	~Stream() {
		if (str != nullptr) fclose(str);
	}
	inline operator FILE* () noexcept { return str; }
	Stream& operator=(const Stream& other) = delete;
	Stream(const Stream& other) = delete;
	Stream(Stream&& other) noexcept {
		str = other.str;
		other.str = nullptr;
	}
	Stream& operator=(Stream&& other) noexcept {
		if (str != nullptr) fclose(str);
		str = other.str;
		other.str = nullptr;
		return *this; 
	}
	Stream(FILE* str) : str(str) {};
};
Stream make_stream(const char* uri, streamMode mode);