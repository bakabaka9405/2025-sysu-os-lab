#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"
#include "sync.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

Semaphore chopstick[5];
Semaphore seat;

void philosopher(void* arg) {
	int philosopher_id = *(int*)arg;
	while (true) {
		printf("philosopher %d: think\n", philosopher_id);
		for (int i = 0; i < 0xfffffff; i++)
			; // 思考
		seat.P();
		printf("philosopher %d: get a seat\n", philosopher_id);
		chopstick[philosopher_id].P();
		printf("philosopher %d: pick up left chopstick\n", philosopher_id);
		for (int i = 0; i < 0xfffffff; i++)
			; // 模拟延迟以复现死锁
		chopstick[(philosopher_id + 1) % 5].P();
		printf("philosopher %d: pick up right chopstick\n", philosopher_id);
		printf("philosopher %d: eat\n", philosopher_id);
		for (int i = 0; i < 0xfffffff; i++)
			; // 吃饭完毕
		chopstick[philosopher_id].V();
		chopstick[(philosopher_id + 1) % 5].V();
		printf("philosopher %d: put down chopsticks\n", philosopher_id);
		seat.V();
		printf("philosopher %d: leave the table\n", philosopher_id);
	}
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();
	for (auto& i : chopstick) i.initialize(1);
	seat.initialize(4);
	int philosopher_id[5] = { 0, 1, 2, 3, 4 };
	for (int i = 0; i < 5; i++) {
		programManager.executeThread(philosopher, (void*)&philosopher_id[i], "philosopher", 1);
	}

	programManager.schedule();
}
