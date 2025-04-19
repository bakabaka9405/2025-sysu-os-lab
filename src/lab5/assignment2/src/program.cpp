#include "program.hpp"
#include "interrupt.hpp"
#include "asm_utils.hpp"
#include "stdio.hpp"
#include "thread.hpp"
#include "stdlib.hpp"
#include "os_modules.hpp"

char PCB_buffer[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_used[MAX_PROGRAM_AMOUNT];				// PCB的分配状态，true表示已经分配，false表示未分配。
int PCB_free_list[MAX_PROGRAM_AMOUNT];			// PCB的空闲链表，存放未分配的PCB的索引。
int PCB_free_count;

int getPCBIndex(PCB* program) {
	return ((int)program - (int)PCB_buffer) / PCB_SIZE;
}

PCB* getPCB(int pid) {
	PCB* program = (PCB*)(PCB_buffer + pid * PCB_SIZE);
	return program;
}

void ProgramManager::initialize() {
	running = nullptr;
	for (auto& i : PCB_used) i = false;
	PCB_free_count = MAX_PROGRAM_AMOUNT;
	for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) PCB_free_list[MAX_PROGRAM_AMOUNT - i - 1] = i;
}

int ProgramManager::executeThread(ThreadFunction function, void* parameter, const char* name, int priority) {
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
	thread->ticks = priority;
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

	allPrograms.push(thread->pid);	 // 将线程加入所有线程列表
	readyPrograms.push(thread->pid); // 将线程加入就绪线程列表

	// 恢复中断
	interruptManager.setInterruptStatus(status);

	return thread->pid;
}

void ProgramManager::schedule() {
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	if (readyPrograms.size() == 0) {
		interruptManager.setInterruptStatus(status);
		return;
	}

	// printf("schedule\n");
	// printf("pid %d name \"%s\"\n", running->pid, running->name);

	if (running->status == ProgramStatus::RUNNING) {
		running->status = ProgramStatus::READY;
		running->ticks = running->priority;
		readyPrograms.push(running->pid);
	}
	else if (running->status == ProgramStatus::DEAD) {
		releasePCB(running);
	}

	int item = readyPrograms.front();
	PCB* next = getPCB(item);
	PCB* cur = running;
	next->status = ProgramStatus::RUNNING;
	running = next;
	readyPrograms.pop();

	asm_switch_thread(cur, next);

	interruptManager.setInterruptStatus(status);
}

void program_exit() {
	PCB* thread = programManager.running;
	thread->status = ProgramStatus::DEAD;

	if (thread->pid) {
		programManager.schedule();
	}
	else {
		interruptManager.disableInterrupt();
		printf("halt in program_exit\n");
		asm_halt();
	}
}

PCB* ProgramManager::allocatePCB() {
	if (PCB_free_count == 0) {
		return nullptr;
	}
	int index = PCB_free_list[--PCB_free_count];
	PCB_used[index] = true;
	return (PCB*)(PCB_buffer + index * PCB_SIZE);
}

void ProgramManager::releasePCB(PCB* program) {
	int index = getPCBIndex(program);
	PCB_used[index] = false;
	PCB_free_list[PCB_free_count++] = index;
}