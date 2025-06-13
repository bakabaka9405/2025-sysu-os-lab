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
int foo() {
	return 1919810;
}
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
}

void second_process() {
	int* p = (int*)memoryManager.allocatePages(AddressPoolType::USER, 1);
	if (!p) {
		printf("Failed to allocate memory\n");
		return;
	}
	*p = 114514;
	int pid = fork();
	if (pid == -1) {
		printf("can not fork\n");
	}
	else {
		if (pid) {
			printf("I am father, fork return: %d\n", pid);
			wait(nullptr);
			printf("child: p = %d, paddr=0x%x\n", *p, memoryManager.vaddr2paddr((uint)p));
		}
		else {
			printf("I am child, fork return: %d, my pid: %d\n", pid, programManager.running->pid);
			printf("child: p = %d, paddr=0x%x\n", *p, memoryManager.vaddr2paddr((uint)p));
			*p = 1919810;
			printf("child: p = %d, paddr=0x%x\n", *p, memoryManager.vaddr2paddr((uint)p));
			exit(0);
		}
	}
}

void first_thread(void*) {
	printf("start process\n");
	programManager.executeProcess((const char*)second_process, 1);
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
	systemService.setSystemCall(3, (void*)syscall_exit);
	systemService.setSystemCall(4, (void*)syscall_wait);
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
