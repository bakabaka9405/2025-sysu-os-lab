#include "syscall.hpp"
#include "interrupt.hpp"
#include "stdlib.hpp"
#include "asm_utils.hpp"
#include "os_modules.hpp"

uint system_call_table[MAX_SYSTEM_CALL];

SystemService::SystemService() {
	initialize();
}

void SystemService::initialize() {
	memset((char*)system_call_table, 0, sizeof(int) * MAX_SYSTEM_CALL);
	// 代码段选择子默认是DPL=0的平坦模式代码段选择子，DPL=3，否则用户态程序无法使用该中断描述符
	interruptManager.setInterruptDescriptor(0x80, (uint32)asm_system_call_handler, 3);
}

bool SystemService::setSystemCall(int index, void* function) {
	system_call_table[index] = (uint)function;
	return true;
}

int fork() {
	return asm_system_call(2);
}

int syscall_fork() {
	return programManager.fork();
}

void exit(int ret) {
	asm_system_call(3, ret);
}

void syscall_exit(int ret) {
	programManager.exit(ret);
}

int wait(int *retval) {
    return asm_system_call(4, (int)retval);
}

int syscall_wait(int *retval) {
    return programManager.wait(retval);
}