#pragma once
#include "os_type.hpp"

extern "C" {
void asm_hello_world();
void asm_lidt(uint32 start, uint16 limit);
void asm_unhandled_interrupt();
void asm_halt();
void asm_out_port(uint16 port, uint8 value);
void asm_in_port(uint16 port, uint8* value);
void asm_enable_interrupt();
void asm_time_interrupt_handler();
int asm_interrupt_status();
void asm_disable_interrupt();
void asm_switch_thread(void* cur, void* next);
void asm_atomic_exchange(uint32* reg, uint32* memory);
void asm_acquire_lock(uint32* lock);
void asm_release_lock(uint32* lock);
void asm_init_page_reg(int* directory);
}
