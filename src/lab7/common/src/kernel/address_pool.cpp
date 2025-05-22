#include "address_pool.hpp"
#include "os_constant.hpp"

AddressPool::AddressPool()
	: startAddress() {
}

// 设置地址池BitMap
void AddressPool::initialize(uint8* bitmap, int length, const void* startAddress) {
	resources.initialize(bitmap, length);
	this->startAddress = startAddress;
}

// 从地址池中分配count个连续页
void* AddressPool::allocate(int count) {
	int start = resources.allocate(count);
	return (start == -1) ? nullptr : (void*)(start * PAGE_SIZE + (uint)startAddress);
}

// 释放若干页的空间
void AddressPool::release(const void* address, int amount) {
	resources.release(int(((uint)address - (uint)startAddress) / PAGE_SIZE), amount);
}
