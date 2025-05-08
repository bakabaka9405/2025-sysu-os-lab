#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"
#include "sync.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

SpinLock aLock;
int cheese_burger = 0;

// Semaphore s;

void a_mother(void* arg) {
	aLock.lock();
	int delay = 0;

	printf("mother: start to make cheese burger, there are %d cheese burger now\n", cheese_burger);
	// make 10 cheese_burger
	cheese_burger += 10;

	printf("mother: oh, I have to hang clothes out.\n");
	// hanging clothes out
	delay = 0xfffffff;
	while (delay)
		--delay;
	// done

	printf("mother: Oh, Jesus! There are %d cheese burgers\n", cheese_burger);
	// aLock.unlock();
	// s.V();
	asm_halt();
}

void a_naughty_boy(void*) {
	// s.P();
	aLock.lock();
	printf("boy   : Look what I found!\n");
	// eat all cheese_burgers out secretly
	cheese_burger -= 10;
	// run away as fast as possible
	aLock.unlock();
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();
	aLock.initialize();
	// s.initialize(0);

	int pid1 = programManager.executeThread(a_mother, nullptr, "first thread", 1);
	int pid2 = programManager.executeThread(a_naughty_boy, nullptr, "second thread", 1);
	if (pid1 == -1 || pid2 == -1) {
		printf("can not execute thread\n");
		asm_halt();
	}

	programManager.schedule();
}
