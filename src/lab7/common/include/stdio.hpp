#pragma once
#include "os_type.hpp"

class STDIO {
public:
	// 打印字符c，颜色color到位置(x,y)
	void putchar(uint x, uint y, uint8 c, uint8 color = 0x07);
	// 打印字符c，颜色color到光标位置
	void putchar(uint8 c, uint8 color = 0x07);
	// 打印整数
	void write_int(int x);
	// 打印十六进制整数
	void write_hex(uint x);
	// 移动光标到一维位置
	void moveCursor(uint position);
	// 移动光标到二维位置
	void moveCursor(uint x, uint y);
	// 获取光标位置
	uint getCursor();
	// 向上滚动一行
	void rollUp();
};

// 打印格式化字符串，颜色默认到光标位置
void printf(const char* format, ...);