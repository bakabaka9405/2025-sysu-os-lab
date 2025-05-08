#pragma once
#include "os_type.hpp"
constexpr int IDT_START_ADDRESS = 0x8880;
constexpr int CODE_SELECTOR = 0x20;
constexpr int MAX_PROGRAM_NAME = 16;
constexpr int MAX_PROGRAM_AMOUNT = 16;
constexpr int PCB_SIZE = 4096; // PCB的大小，4KB。