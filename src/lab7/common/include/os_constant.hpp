#pragma once
#include "os_type.hpp"
constexpr uint IDT_START_ADDRESS = 0x8880;
constexpr uint CODE_SELECTOR = 0x20;
constexpr int MAX_PROGRAM_NAME = 16;
constexpr int MAX_PROGRAM_AMOUNT = 16;
constexpr int PCB_SIZE = 4096;
constexpr uint MEMORY_SIZE_ADDRESS = 0x7c00;
constexpr int PAGE_SIZE = 4096;
constexpr uint BITMAP_START_ADDRESS = 0x10000;
constexpr uint PAGE_DIRECTORY = 0x100000;
constexpr uint KERNEL_VIRTUAL_START = 0xc0100000;