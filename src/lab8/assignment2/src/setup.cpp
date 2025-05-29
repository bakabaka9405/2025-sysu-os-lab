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
void first_process() {
	int pid = fork();

	if (pid == -1) {
		printf("can not fork\n");
	}
	else {
		if (pid) {
			printf("I am father, fork return: %d\n", pid);
		}
		else {
			printf("I am child, fork return: %d, my pid: %d\n", pid, programManager.running->pid);
		}
	}

	asm_halt();
}

void first_thread(void*) {
	printf("start process\n");
	programManager.executeProcess((const char*)first_process, 1);
	printf("first thread finished\n");
	asm_halt();
}

extern "C" void setup_kernel() {
	interruptManager.initialize();
	interruptManager.enableInterrupt();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	systemService.initialize();
	// systemService.setSystemCall(0, (void*)syscall_0);
	systemService.setSystemCall(2, (void*)syscall_fork);
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
