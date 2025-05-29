#include "interrupt.hpp"
#include "program.hpp"
#include "stdio.hpp"
#include "syscall.hpp"
#include "asm_utils.hpp"
#include "memory.hpp"
#include "tss.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;
SystemService systemService;
MemoryManager memoryManager;
TSS tss;

int syscall_0(int first, int second, int third, int forth, int fifth) {
	printf("system call 0: %d, %d, %d, %d, %d\n",
		   first, second, third, forth, fifth);
	return first + second + third + forth + fifth;
}

void first_process() {
	asm_system_call(0, 132, 324, 12, 124);
	asm_halt();
}

void first_thread(void*) {
	printf("start process\n");
	programManager.executeProcess((char*)first_process, 1);
	programManager.executeProcess((char*)first_process, 1);
	programManager.executeProcess((char*)first_process, 1);
	asm_halt();
}

extern "C" void setup_kernel() {
	interruptManager.initialize();
	interruptManager.enableInterrupt();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	systemService.initialize();
	systemService.setSystemCall(0, (void*)syscall_0);
	stdio.initialize();
	memoryManager.initialize();
	programManager.initialize();

	int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
	if (pid == -1) {
		printf("Failed to create first thread\n");
		asm_halt();
	}
	programManager.schedule();
	asm_halt();
}
