#include "stdlib.hpp"

void memset(void* dst, byte value, size_t size) {
	if (!dst || size == 0) return;
	byte* p = (byte*)dst;
	for (size_t i = 0; i < size; ++i) {
		p[i] = value;
	}
}

void memcpy(void* dst, const void* src, size_t size) {
	if (!dst || !src || size == 0) return;
	byte* d = (byte*)dst;
	const byte* s = (const byte*)src;
	for (size_t i = 0; i < size; ++i) {
		d[i] = s[i];
	}
}

void strcpy(char* dst, const char* src) {
	if (!dst || !src) return;
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

int ceil(int x, int y) {
	return (x + y - 1) / y;
}