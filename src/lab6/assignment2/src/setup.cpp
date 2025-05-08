#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"
#include "sync.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

int item[10];
bool available[10];
SpinLock aLock[10];
Semaphore item_count, empty_count;

void producer(void* arg) {
	int producer_id = *(int*)arg;
	int item_id = 0;
	while (true) {
		for (int i = 0; i < 0xfffffff; i++)
			; // 生产一件商品
		bool found = false;
		empty_count.P();
		for (int i = 0; i < 10; i++) {
			if (!available[i]) {
				aLock[i].lock();
				available[i] = true;
				for (int j = 0; j < 0xfffffff; j++)
					; // 把商品放到货柜
				item[i] = ++item_id;
				printf("producer %d: put item %d on %d\n", producer_id, item[i], i);
				aLock[i].unlock();
				item_count.V();
				found = true;
				break;
			}
		}
	}
}

void consumer(void*) {
	while (true) {
		item_count.P();
		for (int i = 0; i < 10; i++) {
			if (available[i]) {
				aLock[i].lock();
				printf("consumer: get item %d\n", item[i]);
				available[i] = false;
				item[i] = 0;
				aLock[i].unlock();
			}
		}
		empty_count.V();
	}
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();
	for (auto& l : aLock) l.initialize();
	item_count.initialize(0);
	empty_count.initialize(10);
	int producer_id1 = 1;
	int producer_id2 = 2;
	int pid1 = programManager.executeThread(producer, &producer_id1, "producer 1", 1);
	int pid2 = programManager.executeThread(producer, &producer_id2, "producer 2", 1);
	int pid3 = programManager.executeThread(consumer, nullptr, "consumer", 1);
	if (pid1 == -1 || pid2 == -1 || pid3 == -1) {
		printf("can not execute thread\n");
		asm_halt();
	}

	programManager.schedule();
}
