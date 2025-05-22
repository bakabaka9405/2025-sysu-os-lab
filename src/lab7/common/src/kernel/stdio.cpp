#include "stdio.hpp"
#include "os_type.hpp"
#include "asm_utils.hpp"
#include "stdarg.hpp"

extern STDIO stdio;

uint8* const screen = (uint8*)0xb8000;

void STDIO::putchar(uint x, uint y, uint8 c, uint8 color) {
	if (x >= 25 || y >= 80) {
		return;
	}

	uint pos = x * 80 + y;
	screen[2 * pos] = c;
	screen[2 * pos + 1] = color;
}

void STDIO::putchar(uint8 c, uint8 color) {
	uint cursor = getCursor();
	screen[2 * cursor] = c;
	screen[2 * cursor + 1] = color;
	cursor++;
	if (cursor == 25 * 80) {
		rollUp();
		cursor = 24 * 80;
	}
	moveCursor(cursor);
}

void STDIO::write_int(int x) {
	if (x < 0) {
		putchar('-');
		x = -x;
	}
	if (x >= 10) write_int(x / 10), x %= 10;
	putchar(char(x + '0'));
}

void STDIO::write_hex(uint x) {
	if (x >= 16) write_hex(x / 16), x %= 16;
	if (x < 10) putchar(char(x + '0'));
	else putchar(char(x - 10 + 'A'));
}

void printf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	for (const char* p = format; *p != '\0'; ++p) {
		if (*p == '%') {
			++p;
			if (*p == 'd') {
				int num = va_arg(args, int);
				stdio.write_int(num);
			}
			else if (*p == 'x') {
				int num = va_arg(args, int);
				stdio.write_hex(num);
			}
			else if (*p == 's') {
				const char* str = va_arg(args, const char*);
				while (*str != '\0') {
					stdio.putchar(*str++);
				}
			}
			else if (*p == 'c') {
				char ch = (char)va_arg(args, int);
				stdio.putchar(ch);
			}
			else if (*p == '%') {
				stdio.putchar('%');
			}
		}
		else if (*p == '\n') { // 换行
			uint cursor = stdio.getCursor();
			cursor += 80 - (cursor % 80);
			if (cursor >= 25 * 80) {
				stdio.rollUp();
				cursor = 24 * 80;
			}
			stdio.moveCursor(cursor);
		}
		else if (*p == '\t') { // 制表符
			uint cursor = stdio.getCursor();
			cursor += 4 - (cursor % 4);
			if (cursor >= 25 * 80) {
				stdio.rollUp();
				cursor = 24 * 80;
			}
			stdio.moveCursor(cursor);
		}
		else {
			stdio.putchar(*p);
		}
	}
}

void STDIO::moveCursor(uint position) {
	if (position >= 80 * 25) {
		return;
	}

	uint8 temp;

	// 处理高8位
	temp = (position >> 8) & 0xff;
	asm_out_port(0x3d4, 0x0e);
	asm_out_port(0x3d5, temp);

	// 处理低8位
	temp = position & 0xff;
	asm_out_port(0x3d4, 0x0f);
	asm_out_port(0x3d5, temp);
}

uint STDIO::getCursor() {
	uint pos;
	uint8 temp;

	pos = 0;
	temp = 0;
	// 处理高8位
	asm_out_port(0x3d4, 0x0e);
	asm_in_port(0x3d5, &temp);
	pos = ((uint)temp) << 8;

	// 处理低8位
	asm_out_port(0x3d4, 0x0f);
	asm_in_port(0x3d5, &temp);
	pos = pos | ((uint)temp);

	return pos;
}

void STDIO::moveCursor(uint x, uint y) {
	if (x >= 25 || y >= 80) {
		return;
	}

	moveCursor(x * 80 + y);
}

void STDIO::rollUp() {
	uint length;
	length = 25 * 80;
	for (uint i = 80; i < length; ++i) {
		screen[2 * (i - 80)] = screen[2 * i];
		screen[2 * (i - 80) + 1] = screen[2 * i + 1];
	}

	for (uint i = 24 * 80; i < length; ++i) {
		screen[2 * i] = ' ';
		screen[2 * i + 1] = 0x07;
	}
}