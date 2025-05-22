#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"
#include "sync.hpp"
#include "os_constant.hpp"
#include "memory.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;
MemoryManager memoryManager;

void first_thread(void*) {
	auto page1 = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
	int* ptr = (int*)page1;
	*ptr = 0x12345678;
	printf("ptr: %x\n", *ptr);
	auto page2 = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
	printf("page1: %x\npage2: %x\n", page1, page2);
	memoryManager.releasePages(AddressPoolType::KERNEL, page1, 1);
	auto page3 = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
	printf("page3: %x\n", page3);
	memoryManager.releasePages(AddressPoolType::KERNEL, page3, 1);
	auto page4 = memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
	printf("page4: %x\n", page4);

	// while (true) {
	// 	void* pages[100];
	// 	for (int i = 1; i <= 100; i++) {
	// 		pages[i - 1] = memoryManager.allocatePages(AddressPoolType::KERNEL, i * 2);
	// 		printf("page%d: %x\n", i, pages[i - 1]);
	// 	}
	// 	for (int i = 1; i <= 100; i++) {
	// 		memoryManager.releasePages(AddressPoolType::KERNEL, pages[i - 1], i * 2);
	// 	}
	// }

	asm_halt();
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();
	memoryManager.openPageMechanism();
	memoryManager.initialize();

	int pid = programManager.executeThread(first_thread, nullptr, "test", 1);

	programManager.schedule();
}
