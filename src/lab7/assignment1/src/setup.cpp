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
	auto page1 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
	auto page2 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
	printf("page1: %x\npage2: %x\n", page1, page2);
	memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page1, 1);
	auto page3 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
	printf("page3: %x\n", page3);
	memoryManager.releasePhysicalPages(AddressPoolType::KERNEL, page3, 1);
	auto page4 = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 2);
	printf("page4: %x\n", page4);

	auto page5 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
	auto page6 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
	printf("page5: %x\npage6: %x\n", page5, page6);
	memoryManager.releasePhysicalPages(AddressPoolType::USER, page5, 1);
	auto page7 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
	printf("page7: %x\n", page7);
	memoryManager.releasePhysicalPages(AddressPoolType::USER, page7, 1);
	auto page8 = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 2);
	printf("page8: %x\n", page8);

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

	int pid = programManager.executeThread(first_thread, nullptr, "test", 0);

	programManager.schedule();
}
