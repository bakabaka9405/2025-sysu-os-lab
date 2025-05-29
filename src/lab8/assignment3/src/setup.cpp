#include "interrupt.hpp"
#include "program.hpp"
#include "stdio.hpp"
#include "syscall.hpp"
#include "asm_utils.hpp"
#include "memory.hpp"
#include "tss.hpp"

InterruptManager interruptManager;
ProgramManager programManager;
STDIO stdio;
SystemService systemService;
MemoryManager memoryManager;
TSS tss;
void first_process() {
	int pid = fork();
	int retval;

	if (pid) {
		pid = fork();
		if (pid) {
			// while ((pid = wait(&retval)) != -1) {
			// 	printf("wait for a child process, pid: %d, return value: %d\n",
			// 		   pid, retval);
			// }

			// printf("all child process exit, programs: %d\n",
			// 	   programManager.allPrograms.size());

			printf("father process exit, pid: %d\n", programManager.running->pid);
			// 直接退出父进程，不使用exit
		}
		else {
			uint32 tmp = 0xffffff;
			while (tmp)
				--tmp;
			printf("exit, pid: %d\n", programManager.running->pid);
			exit(123934);
		}
	}
	else {
		uint32 tmp = 0xffffff;
		while (tmp)
			--tmp;
		printf("exit, pid: %d\n", programManager.running->pid);
		exit(-123);
	}
}

void second_thread(void*) {
	printf("thread exit\n");
	// exit(0);
}

void init() {
	int res = 0;
	while (int pid = wait(&res)) {
		if (pid != -1) {
			printf("process %d exit. return %d. current programs count = %d\n", pid, res, programManager.allPrograms.size());
			for (int pid : programManager.allPrograms) {
				PCB* program = programManager.getPCB(pid);
				printf("pid: %d, name: %s, status: %d\n", pid, program->name, (int)program->status);
			}
		}
	}
}

void first_thread(void*) {
	printf("start process\n");
	programManager.executeProcess((const char*)init, 1);
	programManager.executeProcess((const char*)first_process, 1);
	programManager.executeThread(second_thread, nullptr, "second", 1);
	asm_halt();
}

extern "C" void setup_kernel() {
	interruptManager.initialize();
	interruptManager.enableInterrupt();
	interruptManager.enableTimeInterrupt();
	interruptManager.setTimeInterrupt((void*)asm_time_interrupt_handler);
	systemService.initialize();
	// systemService.setSystemCall(0, (void*)syscall_0);
	systemService.setSystemCall(2, (void*)syscall_fork);
	systemService.setSystemCall(3, (void*)syscall_exit);
	systemService.setSystemCall(4, (void*)syscall_wait);
	stdio.initialize();
	memoryManager.initialize();
	programManager.initialize();

	int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
	if (pid == -1) {
		printf("Failed to create first thread\n");
		asm_halt();
	}
	programManager.schedule();
	asm_halt();
}
