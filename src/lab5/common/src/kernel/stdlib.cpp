#include "stdlib.hpp"

void memset(void* dst, byte value, size_t size) {
	byte* p = (byte*)dst;
	for (size_t i = 0; i < size; ++i) {
		p[i] = value;
	}
}

void memcpy(void* dst, const void* src, size_t size) {
	byte* d = (byte*)dst;
	const byte* s = (const byte*)src;
	for (size_t i = 0; i < size; ++i) {
		d[i] = s[i];
	}
}