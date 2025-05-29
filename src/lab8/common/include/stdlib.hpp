#pragma once
#include "os_type.hpp"

void memset(void* dst, byte value, size_t size);

void memcpy(void* dst, const void* src, size_t size);

void strcpy(char* dst, const char* src);

int ceil(int x, int y);