#pragma once
#include "os_type.hpp"
constexpr uint IDT_START_ADDRESS = 0xc0008880;
constexpr uint CODE_SELECTOR = 0x20;
constexpr uint STACK_SELECTOR = 0x10;
constexpr int MAX_PROGRAM_NAME = 16;
constexpr int MAX_PROGRAM_AMOUNT = 16;
constexpr int PCB_SIZE = 4096;
constexpr uint MEMORY_SIZE_ADDRESS = 0xc0007c00;
constexpr int PAGE_SIZE = 4096;
constexpr uint BITMAP_START_ADDRESS = 0xc0010000;
constexpr uint PAGE_DIRECTORY = 0x100000;
constexpr uint KERNEL_VIRTUAL_START = 0xc0100000;
constexpr int MAX_SYSTEM_CALL = 256;	

constexpr uint USER_CODE_LOW = 0x0000ffff;
constexpr uint USER_CODE_HIGH = 0x00cff800;

constexpr uint USER_DATA_LOW = 0x0000ffff;
constexpr uint USER_DATA_HIGH = 0x00cff200;

constexpr uint USER_STACK_LOW = 0x00000000;
constexpr uint USER_STACK_HIGH = 0x0040f600;

constexpr uint USER_VADDR_START = 0x8048000;