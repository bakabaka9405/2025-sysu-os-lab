#include "mlfq.hpp"
#include "program.hpp"
#include "interrupt.hpp"
#include "asm_utils.hpp"
#include "stdio.hpp"
#include "thread.hpp"
#include "stdlib.hpp"
#include "os_modules.hpp"
#include <cmath>

void MLFQProgramManager::initialize() {
	running = nullptr;
	running_layer = 0;
	initializePCB();
}

int MLFQProgramManager::executeThread(ThreadFunction function, void* parameter, const char* name, int priority) {
	// 关中断，防止创建线程的过程被打断
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	// 分配一页作为PCB
	PCB* thread = allocatePCB();

	if (!thread)
		return -1;

	// 初始化分配的页
	memset(thread, 0, PCB_SIZE);

	for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i) {
		thread->name[i] = name[i];
	}

	thread->status = ProgramStatus::READY;
	thread->priority = priority;
	thread->ticks = priority * weight[0];
	thread->ticksPassedBy = 0;
	thread->pid = getPCBIndex(thread); // 线程的pid为其在PCB_buffer中的索引

	// 线程栈
	thread->stack = (int*)((int)thread + PCB_SIZE);
	thread->stack -= 7;					  // 栈顶从高地址向低地址移动7x4字节，存放
	thread->stack[0] = 0;				  // 保存esi
	thread->stack[1] = 0;				  // 保存edi
	thread->stack[2] = 0;				  // 保存ebx
	thread->stack[3] = 0;				  // 保存ebp
	thread->stack[4] = (int)function;	  // 保存线程函数地址
	thread->stack[5] = (int)program_exit; // 保存线程退出函数地址
	thread->stack[6] = (int)parameter;	  // 保存线程函数参数

	readyPrograms[0].push(thread->pid); // 将线程加入就绪线程列表

	// 恢复中断
	interruptManager.setInterruptStatus(status);

	return thread->pid;
}

void MLFQProgramManager::schedule() {
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	if (running && running->status == ProgramStatus::RUNNING) {
		int next_layer = std::min(4, running_layer + 1);
		running->status = ProgramStatus::READY;
		running->ticks = running->priority * weight[next_layer];
		readyPrograms[next_layer].push(running->pid);
	}
	else if (running && running->status == ProgramStatus::DEAD) {
		releasePCB(running);
	}

	for (int i = 0; i < 5; i++) {
		if (readyPrograms[i].empty()) continue;
		int item = readyPrograms[i].front();
		PCB* next = getPCB(item);
		PCB* cur = running;
		next->status = ProgramStatus::RUNNING;
		running = next;
		printf("switch from %s(layer %d) to %s(layer %d)\n", cur->name, running_layer, next->name, i);
		running_layer = i;
		readyPrograms[i].pop();
		asm_switch_thread(cur, next);
		break;
	}
	interruptManager.setInterruptStatus(status);
}