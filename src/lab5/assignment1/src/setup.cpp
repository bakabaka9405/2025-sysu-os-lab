#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"

STDIO stdio;
InterruptManager interruptManager;

extern "C" void setup_kernel() {
	// 中断处理部件
	// interruptManager.enableInterrupt();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	printf("print percentage: %%\n"
		   "print char \"N\": %c\n"
		   "print string \"Hello World!\": %s\n"
		   "print decimal: \"-1234\": %d\n"
		   "print hexadecimal \"0x7abcdef0\": %x\n",
		   'N', "Hello World!", -1234, 0x7abcdef0);
	// uint a = 1 / 0;
	asm_halt();
}
