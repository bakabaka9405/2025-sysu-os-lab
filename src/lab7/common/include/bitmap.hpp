#pragma once
#include "os_type.hpp"

class BitMap {
public:
	int length;	   // 被管理的资源个数，bitmap的总位数
	uint8* bitmap; // bitmap的起始地址

public:
	// 初始化
	BitMap();

	// 设置BitMap，bitmap=起始地址，length=总位数(即被管理的资源个数)
	void initialize(uint8* bitmap, int length);

	// 获取第index个资源的状态，true=allocated，false=free
	bool get(int index) const;

	// 设置第index个资源的状态，true=allocated，false=free
	void set(int index, bool status);

	// 分配count个连续的资源，若没有则返回-1，否则返回分配的第1个资源单元序号
	int allocate(int count);

	// 释放第index个资源开始的count个资源
	void release(int index, int count);

	// 返回Bitmap存储区域
	uint8* getBitmap() const;

	// 返回Bitmap的大小
	int size() const;

	// 禁止Bitmap之间的赋值
	BitMap(const BitMap&) = delete;
	void operator=(const BitMap&) = delete;
};
