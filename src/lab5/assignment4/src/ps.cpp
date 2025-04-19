#include "ps.hpp"
#include "interrupt.hpp"
#include "asm_utils.hpp"
#include "stdio.hpp"
#include "thread.hpp"
#include "stdlib.hpp"
#include "os_modules.hpp"

void PSProgramManager::initialize() {
	running = nullptr;
	initializePCB();
}

int PSProgramManager::executeThread(ThreadFunction function, void* parameter, const char* name, int priority) {
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
	thread->ticks = priority * 10;
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

	// 将线程加入线程列表
	readyPrograms[readyCount++] = thread->pid;

	// 恢复中断
	interruptManager.setInterruptStatus(status);

	return thread->pid;
}

void PSProgramManager::schedule() {
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	if (readyCount == 0) {
		interruptManager.setInterruptStatus(status);
		return;
	}

	if (!running || running->status == ProgramStatus::DEAD) {
		if (running) releasePCB(running);
		int best = -1;
		for (int i = 0; i < readyCount; i++) {
			PCB* program = getPCB(readyPrograms[i]);
			if (best == -1 || program->priority > getPCB(best)->priority) {
				best = readyPrograms[i];
			}
		}

		if (best != -1) {
			readyPrograms[best] = readyPrograms[--readyCount];
			PCB* next = getPCB(best);
			PCB* cur = running;
			next->status = ProgramStatus::RUNNING;
			running = next;
			asm_switch_thread(cur, next);
		}
	}

	interruptManager.setInterruptStatus(status);
}
