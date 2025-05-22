#pragma once
#include "thread.hpp"
#include "flat_queue.hpp"
class RRProgramManager {
public:
	FlatQueue<int, MAX_PROGRAM_AMOUNT> readyPrograms; // 就绪队列
	PCB* running;									  // 当前执行的线程
public:
	RRProgramManager() = default;
	void initialize();

	// 创建一个线程并放入就绪队列

	// function：线程执行的函数
	// parameter：指向函数的参数的指针
	// name：线程的名称
	// priority：线程的优先级

	// 成功，返回pid；失败，返回-1
	int executeThread(ThreadFunction function, void* parameter, const char* name, int priority);

	void MESA_WakeUp(PCB* program);

	// 执行线程调度
	void schedule();
};
