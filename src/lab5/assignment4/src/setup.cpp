#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

void first_thread(void*) {
	printf("enter first thread\n");
	for (int i = 0; i < 10000000; i++) {}
}

void second_thread(void*) {
	printf("enter second thread\n");
	asm_halt();
}

void schedule_time_interrupt_handler() {
	PCB* cur = programManager.running;
	if (!cur) return;
	++cur->ticksPassedBy;
	if (--cur->ticks <= 0) {
		printf("schedule time interrupt\n");
		printf("before schedule: %s\n", programManager.running->name);
		programManager.schedule();
		printf("after schedule: %s\n", programManager.running->name);
	}
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setCustomTimeInterrupt(schedule_time_interrupt_handler);
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();

	int pid1 = programManager.executeThread(first_thread, nullptr, "first thread", 1);
	int pid2 = programManager.executeThread(second_thread, nullptr, "second thread", 1);
	if (pid1 == -1 || pid2 == -1) {
		printf("can not execute thread\n");
		asm_halt();
	}

	programManager.schedule();
}
