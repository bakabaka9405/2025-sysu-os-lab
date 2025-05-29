#include "memory.hpp"
#include "os_constant.hpp"
#include "stdlib.hpp"
#include "asm_utils.hpp"
#include "stdio.hpp"
#include "program.hpp"
#include "os_modules.hpp"

// 页目录表
uint* toPDE(uint vaddr) {
	return (uint*)(0xfffff000 + (((vaddr & 0xffc00000) >> 22) << 2));
}

// 页表
uint* toPTE(uint vaddr) {
	return (uint*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) << 2));
}

MemoryManager::MemoryManager()
	: totalMemory() {
	initialize();
}

void MemoryManager::initialize() {
	this->totalMemory = 0;
	this->totalMemory = getTotalMemory();

	// 预留的内存
	int usedMemory = 256 * PAGE_SIZE + 0x100000;
	if (this->totalMemory < usedMemory) {
		printf("memory is too small, halt.\n");
		asm_halt();
	}
	// 剩余的空闲的内存
	int freeMemory = this->totalMemory - usedMemory;

	int freePages = freeMemory / PAGE_SIZE;
	int kernelPages = freePages / 2;
	int userPages = freePages - kernelPages;

	int kernelPhysicalStartAddress = usedMemory;
	int userPhysicalStartAddress = usedMemory + kernelPages * PAGE_SIZE;

	uint kernelPhysicalBitMapStart = BITMAP_START_ADDRESS;
	uint userPhysicalBitMapStart = kernelPhysicalBitMapStart + (kernelPages + 7) / 8;
	uint kernelVirtualBitMapStart = userPhysicalBitMapStart + (userPages + 7) / 8;

	kernelPhysical.initialize((uint8*)kernelPhysicalBitMapStart, kernelPages, kernelPhysicalStartAddress);
	userPhysical.initialize((uint8*)userPhysicalBitMapStart, userPages, userPhysicalStartAddress);
	kernelVirtual.initialize((uint8*)kernelVirtualBitMapStart, kernelPages, KERNEL_VIRTUAL_START);

	// printf("total memory: %d bytes ( %d MB )\n",
	// 	   this->totalMemory,
	// 	   this->totalMemory / 1024 / 1024);

	// printf("kernel pool\n"
	// 	   "    start address: 0x%x\n"
	// 	   "    total pages: %d ( %d MB )\n"
	// 	   "    bitmap start address: 0x%x\n",
	// 	   kernelPhysicalStartAddress,
	// 	   kernelPages, kernelPages * PAGE_SIZE / 1024 / 1024,
	// 	   kernelPhysicalBitMapStart);

	// printf("user pool\n"
	// 	   "    start address: 0x%x\n"
	// 	   "    total pages: %d ( %d MB )\n"
	// 	   "    bit map start address: 0x%x\n",
	// 	   userPhysicalStartAddress,
	// 	   userPages, userPages * PAGE_SIZE / 1024 / 1024,
	// 	   userPhysicalBitMapStart);
}

uint MemoryManager::allocatePhysicalPages(enum AddressPoolType type, const int count) {
	uint start = 0;

	if (type == AddressPoolType::KERNEL) {
		start = kernelPhysical.allocate(count);
	}
	else if (type == AddressPoolType::USER) {
		start = userPhysical.allocate(count);
	}

	return start;
}

void MemoryManager::releasePhysicalPages(enum AddressPoolType type, uint startAddress, const int count) {
	if (type == AddressPoolType::KERNEL) {
		kernelPhysical.release(startAddress, count);
	}
	else if (type == AddressPoolType::USER) {
		userPhysical.release(startAddress, count);
	}
}

int MemoryManager::getTotalMemory() {

	if (!this->totalMemory) {
		int memory = *((int*)MEMORY_SIZE_ADDRESS);
		// ax寄存器保存的内容
		int low = memory & 0xffff;
		// bx寄存器保存的内容
		int high = (memory >> 16) & 0xffff;

		this->totalMemory = low * 1024 + high * 64 * 1024;
	}

	return this->totalMemory;
}

void MemoryManager::openPageMechanism() {
	// 页目录表指针
	int* directory = (int*)PAGE_DIRECTORY;
	// 线性地址0~4MB对应的页表
	int* page = (int*)(PAGE_DIRECTORY + PAGE_SIZE);

	// 初始化页目录表
	memset(directory, 0, PAGE_SIZE);
	// 初始化线性地址0~4MB对应的页表
	memset(page, 0, PAGE_SIZE);

	int address = 0;
	// 将线性地址0~1MB恒等映射到物理地址0~1MB
	for (int i = 0; i < 256; ++i) {
		// U/S = 1, R/W = 1, P = 1
		page[i] = address | 0x7;
		address += PAGE_SIZE;
	}

	// 初始化页目录项

	// 0~1MB
	directory[0] = ((int)page) | 0x07;
	// 3GB的内核空间
	directory[768] = directory[0];
	// 最后一个页目录项指向页目录表
	directory[1023] = ((int)directory) | 0x7;

	// 初始化cr3，cr0，开启分页机制
	asm_init_page_reg(directory);

	printf("open page mechanism\n");
}

uint MemoryManager::allocatePages(AddressPoolType type, int count) {
	// 第一步：从虚拟地址池中分配若干虚拟页
	auto virtualAddress = allocateVirtualPages(type, count);
	if (!virtualAddress) return 0;

	uint vaddress = (uint)virtualAddress;

	// 依次为每一个虚拟页指定物理页
	for (int i = 0; i < count; ++i, vaddress += PAGE_SIZE) {
		bool flag = false;
		// 第二步：从物理地址池中分配一个物理页
		auto physicalPageAddress = allocatePhysicalPages(type, 1);

		if (physicalPageAddress) {
			// 第三步：为虚拟页建立页目录项和页表项，使虚拟页内的地址经过分页机制变换到物理页内。
			flag = connectPhysicalVirtualPage(vaddress, physicalPageAddress);
		}
		if (!flag) {
			// 分配物理页失败，释放之前分配的虚拟页
			// 前i个页表已经指定了物理页
			releasePages(type, virtualAddress, i);
			// 剩余的页表未指定物理页
			releaseVirtualPages(type, virtualAddress + i * PAGE_SIZE, count - i);
			return 0;
		}
	}

	return virtualAddress;
}

uint MemoryManager::allocateVirtualPages(AddressPoolType type, int count) {
	if (type == AddressPoolType::KERNEL) {
		return kernelVirtual.allocate(count);
	}
	else {
		return programManager.running->userVirtual.allocate(count);
	}
}

bool MemoryManager::connectPhysicalVirtualPage(uint virtualAddress, uint physicalAddress) {
	// 计算虚拟地址对应的页目录项和页表项
	auto pde = (uint*)toPDE(virtualAddress);
	auto pte = (uint*)toPTE(virtualAddress);

	// 页目录项无对应的页表，先分配一个页表
	if (!(*pde & 0x00000001)) {
		// 从内核物理地址空间中分配一个页表
		auto page = allocatePhysicalPages(AddressPoolType::KERNEL, 1);
		if (!page) return false;

		// 使页目录项指向页表
		*pde = (uint)page | 0x7;
		// 初始化页表
		auto pagePtr = (void*)((uint)pte & 0xfffff000);
		memset(pagePtr, 0, PAGE_SIZE);
	}

	// 使页表项指向物理页
	*pte = (uint)physicalAddress | 0x7;

	return true;
}

void MemoryManager::releasePages(AddressPoolType type, uint virtualAddress, int count) {
	uint vaddr = virtualAddress;

	for (int i = 0; i < count; ++i, vaddr += PAGE_SIZE) {
		releasePhysicalPages(type, vaddr2paddr(vaddr), 1);

		// 设置页表项为不存在，防止释放后被再次使用
		*toPTE(vaddr) = 0;
	}

	releaseVirtualPages(type, virtualAddress, count);
}

uint MemoryManager::vaddr2paddr(uint vaddr) {
	auto page = *toPTE(vaddr) & 0xfffff000;
	auto offset = vaddr & 0xfff;
	return page + offset;
}

void MemoryManager::releaseVirtualPages(AddressPoolType type, uint vaddr, int count) {
	if (type == AddressPoolType::KERNEL) {
		kernelVirtual.release(vaddr, count);
	}
	else {
		programManager.running->userVirtual.release(vaddr, count);
	}
}