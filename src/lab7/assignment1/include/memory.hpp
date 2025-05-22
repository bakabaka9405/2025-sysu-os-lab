#pragma once

#include "address_pool.hpp"

enum AddressPoolType {
	USER,
	KERNEL
};

class MemoryManager {
public:
	// 可管理的内存容量
	int totalMemory;
	// 内核物理地址池
	AddressPool kernelPhysical;
	// 用户物理地址池
	AddressPool userPhysical;

public:
	MemoryManager();

	// 初始化地址池
	void initialize();

	// 从type类型的物理地址池中分配count个连续的页
	// 成功，返回起始地址；失败，返回0
	void* allocatePhysicalPages(AddressPoolType type, int count);

	// 释放从paddr开始的count个物理页
	void releasePhysicalPages(AddressPoolType type, void* startAddress, int count);

	// 获取内存总容量
	int getTotalMemory();

	// 开启分页机制
	void openPageMechanism();
};
