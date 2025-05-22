#include "asm_utils.hpp"
#include "interrupt.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "thread.hpp"
#include "os_constant.hpp"
InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;

class Allocator {
public:
	int pageCount;
	int PhysicalStartAddress;
	uint8* bitmap;

public:
	Allocator() = default;
	void initialize() {
		int memory = *((int*)MEMORY_SIZE_ADDRESS);
		int low = memory & 0xffff;
		int high = (memory >> 16) & 0xffff;
		int totalMemory = low * 1024 + high * 64 * 1024;
		int usedMemory = 256 * PAGE_SIZE + 0x100000;
		int freeMemory = totalMemory - usedMemory;

		pageCount = freeMemory / PAGE_SIZE;

		PhysicalStartAddress = usedMemory;

		bitmap = (uint8*)BITMAP_START_ADDRESS;

		printf("totalMemory: %d MB\n", totalMemory / 1024 / 1024);
		printf("freeMemory: %d MB\n", freeMemory / 1024 / 1024);
		printf("pageCount: %d\n", pageCount);
		printf("PhysicalStartAddress: %x\n", PhysicalStartAddress);
		printf("bitmapAddress: %x\n", bitmap);

		for (int i = 0; i < pageCount; i++)
			bitmap[i] = 0;
	}
	void* allocate(int count) const {
		if (count <= 0) return nullptr;
		// best-fit
		int best_index = -1, best_count = 0;
		for (int i = 0; i < pageCount;) {
			int empty = 0;
			while (i < pageCount && bitmap[i] == 1) {
				i++;
			}
			while (i < pageCount && bitmap[i] == 0) {
				empty++;
				i++;
			}
			if (empty < count) continue;
			if (best_index == -1 || empty < best_count) {
				best_index = i - empty;
				best_count = empty;
			}
		}
		if (best_index == -1) return nullptr;
		for (int i = best_index; i < best_index + count; i++) {
			bitmap[i] = 1;
		}
		return (void*)(PhysicalStartAddress + best_index * PAGE_SIZE);
	}

	void deallocate(const void* address, int count) const {
		if (count <= 0) return;
		int index = ((int)address - PhysicalStartAddress) / PAGE_SIZE;
		for (int i = index; i < index + count; i++) {
			bitmap[i] = 0;
		}
	}
} allocator;

void first_thread(void*) {
	printf("first thread\n");
	int* p1 = (int*)allocator.allocate(1000);
	int* p2 = (int*)allocator.allocate(1000);
	int* p3 = (int*)allocator.allocate(1000);
	printf("p1: %x\n", p1);
	printf("p2: %x\n", p2);
	printf("p3: %x\n", p3);
	allocator.deallocate(p2, 1000);
	int* p4 = (int*)allocator.allocate(200);
	printf("p4: %x\n", p4);
	allocator.deallocate(p1, 1000);
	int* p5 = (int*)allocator.allocate(500);
	printf("p5: %x\n", p5);
	int* p6 = (int*)allocator.allocate(1500);
	printf("p6: %x\n", p6);
	asm_halt();
}

extern "C" void setup_kernel() {
	programManager.initialize();
	// 中断管理器
	interruptManager.initialize();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	interruptManager.enableInterrupt();
	allocator.initialize();

	programManager.executeThread(first_thread, nullptr, "test", 1);

	programManager.schedule();
}
