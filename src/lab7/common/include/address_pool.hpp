#pragma once
#include "bitmap.hpp"
#include "os_type.hpp"

class AddressPool {
public:
	BitMap resources;
	const void* startAddress;

public:
	AddressPool();

	// 初始化地址池
	void initialize(uint8* bitmap, int length, const void* startAddress);

	// 从地址池中分配count个连续页，成功则返回第一个页的地址，失败则返回-1
	void* allocate(int count);

	// 释放若干页的空间
	void release(const void* address, int amount);
};
