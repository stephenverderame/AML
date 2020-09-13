#define _CRT_SECURE_NO_WARNINGS
#include "Stream.h"
#include <string>
Stream make_stream(const char* uri, streamMode mode) {
	if (strstr(uri, "std"))
		return { mode == streamMode::input ? stdin : stdout };
	else {
		return { fopen(uri, mode == streamMode::input ? "r" : "w") };
	}

}