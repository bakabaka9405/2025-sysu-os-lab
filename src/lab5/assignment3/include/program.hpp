#pragma once
#include "thread.hpp"
#include "flat_queue.hpp"
class ProgramManager {
public:
	FlatQueue<int, MAX_PROGRAM_AMOUNT> allPrograms;	  // 所有状态的线程/进程的队列
	FlatQueue<int, MAX_PROGRAM_AMOUNT> readyPrograms; // 就绪队列
	PCB* running;									  // 当前执行的线程
public:
	ProgramManager()=default;
	void initialize();

	// 创建一个线程并放入就绪队列

	// function：线程执行的函数
	// parameter：指向函数的参数的指针
	// name：线程的名称
	// priority：线程的优先级

	// 成功，返回pid；失败，返回-1
	int executeThread(ThreadFunction function, void* parameter, const char* name, int priority);

	// 分配一个PCB
	PCB* allocatePCB();
	// 归还一个PCB
	// program：待释放的PCB
	void releasePCB(PCB* program);

	// 执行线程调度
	void schedule();
};

PCB* getPCB(int pid);

void program_exit();
