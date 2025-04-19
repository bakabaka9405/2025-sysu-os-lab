#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

void first_thread(void*) {
	for (int i = 0;; i++) {
		printf("from %s: %d, %d \n", programManager.running->name, i, 1);
		for (int j = 0; j < 100000000; j++) {}
		printf("from %s: %d, %d \n", programManager.running->name, i, 2);
		for (int j = 0; j < 100000000; j++) {}
		printf("from %s: %d, %d \n", programManager.running->name, i, 3);
		for (int j = 0; j < 100000000; j++) {}
	}
}

void second_thread(void*) {
	asm_halt();
}

void schedule_time_interrupt_handler() {
	PCB* cur = programManager.running;
	if (!cur) return;
	++cur->ticksPassedBy;
	if (--cur->ticks <= 0){
		programManager.schedule();
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

	int pid1 = programManager.executeThread(first_thread, nullptr, "first thread", 2);
	int pid2 = programManager.executeThread(second_thread, nullptr, "second thread", 2);
	if (pid1 == -1 || pid2 == -1) {
		printf("can not execute thread\n");
		asm_halt();
	}
	int item = programManager.readyPrograms.front();
	PCB* firstThread = getPCB(item);
	firstThread->status = ProgramStatus::RUNNING;
	programManager.readyPrograms.pop();
	programManager.running = firstThread;
	asm_switch_thread(nullptr, firstThread);

	asm_halt();
}
