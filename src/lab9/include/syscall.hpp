#pragma once
#include "os_constant.hpp"

class SystemService {
public:
	SystemService();
	void initialize();
	// 设置系统调用，index=系统调用号，function=处理第index个系统调用函数的地址
	bool setSystemCall(int index, void* function);
};

// 第0个系统调用
int syscall_0(int first, int second, int third, int forth, int fifth);

int fork();
int syscall_fork();

void exit(int ret);
void syscall_exit(int ret);

int wait(int *retval);
int syscall_wait(int *retval);