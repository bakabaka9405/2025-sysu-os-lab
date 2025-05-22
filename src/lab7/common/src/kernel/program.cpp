#include "program.hpp"
#include "os_modules.hpp"
#include "asm_utils.hpp"
inline char PCB_buffer[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
inline bool PCB_used[MAX_PROGRAM_AMOUNT];			   // PCB的分配状态，true表示已经分配，false表示未分配。
inline int PCB_free_list[MAX_PROGRAM_AMOUNT];		   // PCB的空闲链表，存放未分配的PCB的索引。
inline int PCB_free_count;

void initializePCB() {
	for (auto& i : PCB_used) i = false;
	PCB_free_count = MAX_PROGRAM_AMOUNT;
	for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) PCB_free_list[MAX_PROGRAM_AMOUNT - i - 1] = i;
}

inline int getPCBIndex(PCB* program) {
	return ((int)program - (int)PCB_buffer) / PCB_SIZE;
}

PCB* getPCB(int pid) {
	PCB* program = (PCB*)(PCB_buffer + pid * PCB_SIZE);
	return program;
}

PCB* allocatePCB() {
	if (PCB_free_count == 0) {
		return nullptr;
	}
	int index = PCB_free_list[--PCB_free_count];
	PCB_used[index] = true;
	return (PCB*)(PCB_buffer + index * PCB_SIZE);
}

void releasePCB(PCB* program) {
	if (program == nullptr) return;
	int index = getPCBIndex(program);
	PCB_used[index] = false;
	PCB_free_list[PCB_free_count++] = index;
}

void program_exit() {
	PCB* thread = programManager.running;
	thread->status = ProgramStatus::DEAD;
	// printf("exit pid=%d name \"%s\"\n", thread->pid, thread->name);
	if (thread->pid >= 0) {
		programManager.schedule();
	}
	else {
		interruptManager.disableInterrupt();
		printf("halt in program_exit\n");
		asm_halt();
	}
}

