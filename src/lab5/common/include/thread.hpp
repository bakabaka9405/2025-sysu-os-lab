#pragma once

#include "os_type.hpp"
#include "os_constant.hpp"

typedef void (*ThreadFunction)(void*);

enum class ProgramStatus {
	CREATED,
	RUNNING,
	READY,
	BLOCKED,
	DEAD
};

struct PCB {
	int* stack;						 // 栈指针，用于调度时保存esp
	char name[MAX_PROGRAM_NAME + 1]; // 线程名
	ProgramStatus status;			 // 线程的状态
	int priority;					 // 线程优先级
	int pid;						 // 线程pid
	int ticks;						 // 线程时间片总时间
	int ticksPassedBy;				 // 线程已执行时间
};
