#pragma once
#include "thread.hpp"
#include "flat_queue.hpp"

class ProgramManager {
public:
	FlatQueue<int, MAX_PROGRAM_AMOUNT> allPrograms;	  // 所有线程的队列
	FlatQueue<int, MAX_PROGRAM_AMOUNT> readyPrograms; // 就绪队列
	PCB* running{};									  // 当前执行的线程
public:
	ProgramManager() = default;
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

	void initializePCB();

	PCB* getPCB(int pid);

	int getPCBIndex(PCB* program);

	// 分配一个PCB
	PCB* allocatePCB();
	// 归还一个PCB
	// program：待释放的PCB
	void releasePCB(PCB* program);

	// 初始化TSS
	void initializeTSS();

	// 创建进程
	int executeProcess(const char* filename, int priority);

	// 创建用户页目录表
	uint createProcessPageDirectory();

	// 创建用户地址池
	bool createUserVirtualPool(PCB* process);

	// 切换页目录表，实现虚拟地址空间的切换
	void activateProgramPage(PCB* program);

	int fork();

	bool copyProcess(PCB* parent, PCB* child);

	void exit(int ret);

	int wait(int* retval);
};

void program_exit();

void load_process(const char* filename);