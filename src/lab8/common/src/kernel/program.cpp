#include "program.hpp"
#include "os_modules.hpp"
#include "asm_utils.hpp"
#include "stdlib.hpp"
#include "os_constant.hpp"
#include "address_pool.hpp"
#include "process.hpp"
#include "thread.hpp"
#include "stdlib.hpp"
#include "syscall.hpp"
inline char PCB_buffer[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
inline bool PCB_used[MAX_PROGRAM_AMOUNT];			   // PCB的分配状态，true表示已经分配，false表示未分配。
inline int PCB_free_list[MAX_PROGRAM_AMOUNT];		   // PCB的空闲链表，存放未分配的PCB的索引。
inline int PCB_free_count;

inline int USER_CODE_SELECTOR;	// 用户代码段选择子
inline int USER_DATA_SELECTOR;	// 用户数据段选择子
inline int USER_STACK_SELECTOR; // 用户栈段选择子

void ProgramManager::initializePCB() {
	for (auto& i : PCB_used) i = false;
	PCB_free_count = MAX_PROGRAM_AMOUNT;
	for (int i = 0; i < MAX_PROGRAM_AMOUNT; i++) PCB_free_list[MAX_PROGRAM_AMOUNT - i - 1] = i;

	// 初始化用户代码段、数据段和栈段
	int selector;

	selector = asm_add_global_descriptor(USER_CODE_LOW, USER_CODE_HIGH);
	USER_CODE_SELECTOR = (selector << 3) | 0x3;

	selector = asm_add_global_descriptor(USER_DATA_LOW, USER_DATA_HIGH);
	USER_DATA_SELECTOR = (selector << 3) | 0x3;

	selector = asm_add_global_descriptor(USER_STACK_LOW, USER_STACK_HIGH);
	USER_STACK_SELECTOR = (selector << 3) | 0x3;
}

inline int ProgramManager::getPCBIndex(PCB* program) {
	return ((int)program - (int)PCB_buffer) / PCB_SIZE;
}

PCB* ProgramManager::getPCB(int pid) {
	PCB* program = (PCB*)(PCB_buffer + pid * PCB_SIZE);
	return program;
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
	if (program == nullptr) return;
	printf("release PCB pid=%d name \"%s\"\n", program->pid, program->name);
	int index = getPCBIndex(program);
	PCB_used[index] = false;
	PCB_free_list[PCB_free_count++] = index;
	if (!allPrograms.erase(program->pid)) {
		printf("release PCB error, pid=%d not found in allPrograms\n", program->pid);
	}
}

void program_exit() {
	PCB* thread = programManager.running;
	thread->status = ProgramStatus::DEAD;
	if (thread->pid >= 0) {
		programManager.schedule();
	}
	else {
		interruptManager.disableInterrupt();
		printf("halt in program_exit\n");
		asm_halt();
	}
}

void ProgramManager::initialize() {
	running = nullptr;
	initializePCB();
	initializeTSS();
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

	if (name) {
		for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i) {
			thread->name[i] = name[i];
		}
	}

	thread->status = ProgramStatus::READY;
	thread->priority = priority;
	thread->ticks = priority * 2;
	thread->ticksPassedBy = 0;
	thread->pid = getPCBIndex(thread); // 线程的pid为其在PCB_buffer中的索引

	// 线程栈
	thread->stack = (int*)((int)thread + PCB_SIZE - sizeof(ProcessStartStack));
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

	if (running) {
		if (running->status == ProgramStatus::RUNNING) {
			running->status = ProgramStatus::READY;
			running->ticks = running->priority * 2;
			readyPrograms.push(running->pid);
		}
		else if (running->status == ProgramStatus::DEAD) {
			// 回收线程，子进程留到父进程回收
			if (!running->pageDirectoryAddress) {
				releasePCB(running);
			}
		}
	}

	int item = readyPrograms.front();
	readyPrograms.pop();
	PCB* next = getPCB(item);
	PCB* cur = running;
	next->status = ProgramStatus::RUNNING;
	running = next;

	activateProgramPage(next);
	asm_switch_thread(cur, next);

	interruptManager.setInterruptStatus(status);
}

void ProgramManager::MESA_WakeUp(PCB* program) {
	program->status = ProgramStatus::READY;
	readyPrograms.push(getPCBIndex(program));
}

void ProgramManager::initializeTSS() {

	auto size = sizeof(TSS);
	auto address = (uint)&tss;

	memset((char*)address, 0, size);
	tss.ss0 = STACK_SELECTOR; // 内核态堆栈段选择子

	uint low, high, limit;

	limit = size - 1;
	low = (address << 16) | (limit & 0xff);
	// DPL = 0
	high = (address & 0xff000000) | ((address & 0x00ff0000) >> 16) | ((limit & 0xff00) << 16) | 0x00008900;

	int selector = asm_add_global_descriptor(low, high);
	// RPL = 0
	asm_ltr(selector << 3);
	// printf("addr: %x\n", address + size);
	tss.ioMap = int(address + size);
}

int ProgramManager::executeProcess(const char* filename, int priority) {
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	// 在线程创建的基础上初步创建进程的PCB
	int pid = executeThread((ThreadFunction)load_process, (void*)filename, nullptr, priority);
	if (pid == -1) {
		interruptManager.setInterruptStatus(status);
		return -1;
	}

	auto process = getPCB(pid);

	// 创建进程的页目录表
	if (!(process->pageDirectoryAddress = createProcessPageDirectory())) {
		process->status = ProgramStatus::DEAD;
		interruptManager.setInterruptStatus(status);
		return -1;
	}

	// 创建进程的虚拟地址池
	if (!createUserVirtualPool(process)) {
		process->status = ProgramStatus::DEAD;
		interruptManager.setInterruptStatus(status);
		return -1;
	}

	interruptManager.setInterruptStatus(status);

	return pid;
}

uint ProgramManager::createProcessPageDirectory() {
	// 从内核地址池中分配一页存储用户进程的页目录表
	uint vaddr = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
	if (!vaddr) {
		// printf("can not create page from kernel\n");
		return 0;
	}

	memset((char*)vaddr, 0, PAGE_SIZE);

	// 复制内核目录项到虚拟地址的高1GB
	int* src = (int*)(0xfffff000 + 0x300 * 4);
	int* dst = (int*)(vaddr + 0x300 * 4);
	for (int i = 0; i < 256; ++i) {
		dst[i] = src[i];
	}

	// 用户进程页目录表的最后一项指向用户进程页目录表本身
	((uint*)vaddr)[1023] = memoryManager.vaddr2paddr(vaddr) | 0x7;

	return vaddr;
}

bool ProgramManager::createUserVirtualPool(PCB* process) {
	int sourcesCount = (0xc0000000 - USER_VADDR_START) / PAGE_SIZE;
	int bitmapLength = (sourcesCount + 7) / 8;

	// 计算位图所占的页数
	int pagesCount = (bitmapLength + PAGE_SIZE - 1) / PAGE_SIZE;

	uint start = memoryManager.allocatePages(AddressPoolType::KERNEL, pagesCount);

	if (!start) {
		return false;
	}

	memset((void*)start, 0, PAGE_SIZE * pagesCount);
	(process->userVirtual).initialize((uint8*)start, bitmapLength, USER_VADDR_START);

	return true;
}

void load_process(const char* filename) {
	interruptManager.disableInterrupt();

	PCB* process = programManager.running;
	auto interruptStack = (ProcessStartStack*)((uint)process + PAGE_SIZE - sizeof(ProcessStartStack));

	interruptStack->edi = 0;
	interruptStack->esi = 0;
	interruptStack->ebp = 0;
	interruptStack->esp_dummy = 0;
	interruptStack->ebx = 0;
	interruptStack->edx = 0;
	interruptStack->ecx = 0;
	interruptStack->eax = 0;
	interruptStack->gs = 0;

	interruptStack->fs = USER_DATA_SELECTOR;
	interruptStack->es = USER_DATA_SELECTOR;
	interruptStack->ds = USER_DATA_SELECTOR;

	interruptStack->eip = (int)filename;
	interruptStack->cs = USER_CODE_SELECTOR;				  // 用户模式平坦模式
	interruptStack->eflags = (0 << 12) | (1 << 9) | (1 << 1); // IOPL, IF = 1 开中断, MBS = 1 默认

	interruptStack->esp = memoryManager.allocatePages(AddressPoolType::USER, 1);
	if (interruptStack->esp == 0) {
		printf("can not build process!\n");
		process->status = ProgramStatus::DEAD;
		asm_halt();
	}
	interruptStack->esp += PAGE_SIZE;

	// 设置进程返回地址
	int* userStack = (int*)interruptStack->esp;
	userStack -= 3;
	userStack[0] = (int)exit;
	userStack[1] = 0;
	userStack[2] = 0;

	interruptStack->esp = (int)userStack;

	interruptStack->ss = USER_STACK_SELECTOR;

	asm_start_process((uint)interruptStack);
}

void ProgramManager::activateProgramPage(PCB* program) {
	uint paddr = PAGE_DIRECTORY;

	if (program->pageDirectoryAddress) {
		tss.esp0 = (int)program + PAGE_SIZE;
		paddr = memoryManager.vaddr2paddr(program->pageDirectoryAddress);
	}

	asm_update_cr3(paddr);
}

int ProgramManager::fork() {
	bool status = interruptManager.getInterruptStatus();
	interruptManager.disableInterrupt();

	PCB* parent = this->running;
	if (!parent->pageDirectoryAddress) {
		interruptManager.setInterruptStatus(status);
		return -1;
	}

	int pid = executeProcess(nullptr, 0);
	if (pid == -1) {
		interruptManager.setInterruptStatus(status);
		return -1;
	}

	PCB* child = getPCB(pid);
	if (!copyProcess(parent, child)) {
		child->status = ProgramStatus::DEAD;
		interruptManager.setInterruptStatus(status);
		return -1;
	}
	interruptManager.setInterruptStatus(status);
	return pid;
}

bool ProgramManager::copyProcess(PCB* parent, PCB* child) {
	// 复制进程0级栈
	auto childpss = (ProcessStartStack*)((int)child + PAGE_SIZE - sizeof(ProcessStartStack));
	auto parentpss = (ProcessStartStack*)((int)parent + PAGE_SIZE - sizeof(ProcessStartStack));
	memcpy(childpss, parentpss, sizeof(ProcessStartStack));
	// 设置子进程的返回值为0
	childpss->eax = 0;

	// 准备执行asm_switch_thread的栈的内容
	child->stack = (int*)childpss - 7;
	child->stack[0] = 0;
	child->stack[1] = 0;
	child->stack[2] = 0;
	child->stack[3] = 0;
	child->stack[4] = (int)asm_start_process;
	child->stack[5] = 0;			 // asm_start_process 返回地址
	child->stack[6] = (int)childpss; // asm_start_process 参数
	// 设置子进程的PCB
	child->status = ProgramStatus::READY;
	child->parentPid = parent->pid;
	child->priority = parent->priority;
	child->ticks = parent->ticks;
	child->ticksPassedBy = parent->ticksPassedBy;
	strcpy(child->name, parent->name);

	// 复制用户虚拟地址池
	int bitmapLength = parent->userVirtual.resources.length;
	int bitmapBytes = ceil(bitmapLength, 8);
	memcpy(child->userVirtual.resources.bitmap, parent->userVirtual.resources.bitmap, bitmapBytes);

	// 从内核中分配一页作为中转页
	uint buffer = memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
	if (!buffer) {
		child->status = ProgramStatus::DEAD;
		return false;
	}

	// 子进程页目录表物理地址
	uint childPageDirPaddr = memoryManager.vaddr2paddr(child->pageDirectoryAddress);
	// 父进程页目录表物理地址
	uint parentPageDirPaddr = memoryManager.vaddr2paddr(parent->pageDirectoryAddress);
	// 子进程页目录表指针(虚拟地址)
	uint* childPageDir = (uint*)child->pageDirectoryAddress;
	// 父进程页目录表指针(虚拟地址)
	uint* parentPageDir = (uint*)parent->pageDirectoryAddress;

	// 子进程页目录表初始化
	memset((void*)child->pageDirectoryAddress, 0, 768 * 4);

	// 复制页目录表
	for (int i = 0; i < 768; ++i) {
		// 无对应页表
		if (!(parentPageDir[i] & 0x1)) {
			continue;
		}

		// 从用户物理地址池中分配一页，作为子进程的页目录项指向的页表
		uint paddr = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
		if (!paddr) {
			child->status = ProgramStatus::DEAD;
			return false;
		}
		// 页目录项
		uint pde = parentPageDir[i];
		// 构造页表的起始虚拟地址
		uint* pageTableVaddr = (uint*)(0xffc00000 + (i << 12));

		asm_update_cr3(childPageDirPaddr); // 进入子进程虚拟地址空间

		childPageDir[i] = (pde & 0x00000fff) | paddr;
		memset(pageTableVaddr, 0, PAGE_SIZE);

		asm_update_cr3(parentPageDirPaddr); // 回到父进程虚拟地址空间
	}

	// 复制页表和物理页
	for (int i = 0; i < 768; ++i) {
		// 无对应页表
		if (!(parentPageDir[i] & 0x1)) {
			continue;
		}

		// 计算页表的虚拟地址
		uint* pageTableVaddr = (uint*)(0xffc00000 + (i << 12));

		// 复制物理页
		for (int j = 0; j < 1024; ++j) {
			// 无对应物理页
			if (!(pageTableVaddr[j] & 0x1)) {
				continue;
			}

			// 从用户物理地址池中分配一页，作为子进程的页表项指向的物理页
			uint paddr = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
			if (!paddr) {
				child->status = ProgramStatus::DEAD;
				return false;
			}

			// 构造物理页的起始虚拟地址
			void* pageVaddr = (void*)((i << 22) + (j << 12));
			// 页表项
			uint pte = pageTableVaddr[j];
			// 复制出父进程物理页的内容到中转页
			memcpy((void*)buffer, pageVaddr, PAGE_SIZE);

			asm_update_cr3(childPageDirPaddr); // 进入子进程虚拟地址空间

			pageTableVaddr[j] = (pte & 0x00000fff) | paddr;
			// 从中转页中复制到子进程的物理页
			memcpy(pageVaddr, (void*)buffer, PAGE_SIZE);

			asm_update_cr3(parentPageDirPaddr); // 回到父进程虚拟地址空间
		}
	}

	// 归还从内核分配的中转页
	memoryManager.releasePages(AddressPoolType::KERNEL, buffer, 1);
	return true;
}

void ProgramManager::exit(int ret) {
	// 关中断
	interruptManager.disableInterrupt();
	// 第一步，标记PCB状态为`DEAD`并放入返回值。
	PCB* program = this->running;
	program->retValue = ret;
	program->status = ProgramStatus::DEAD;

	printf("exit program pid=%d name \"%s\" ret=%d\n", program->pid, program->name, ret);

	uint *pageDir, *page;
	uint paddr;

	// 第二步，如果PCB标识的是进程，则释放进程所占用的物理页、页表、页目录表和虚拟地址池bitmap的空间。
	if (program->pageDirectoryAddress) {
		pageDir = (uint*)program->pageDirectoryAddress;
		for (int i = 0; i < 768; ++i) {
			if (!(pageDir[i] & 0x1)) {
				continue;
			}

			page = (uint*)(0xffc00000 + (i << 12));

			for (int j = 0; j < 1024; ++j) {
				if (!(page[j] & 0x1)) {
					continue;
				}

				paddr = memoryManager.vaddr2paddr((i << 22) + (j << 12));
				memoryManager.releasePhysicalPages(AddressPoolType::USER, paddr, 1);
			}

			paddr = memoryManager.vaddr2paddr((int)page);
			memoryManager.releasePhysicalPages(AddressPoolType::USER, paddr, 1);
		}

		memoryManager.releasePages(AddressPoolType::KERNEL, (int)pageDir, 1);

		int bitmapBytes = ceil(program->userVirtual.resources.length, 8);
		int bitmapPages = ceil(bitmapBytes, PAGE_SIZE);

		memoryManager.releasePages(AddressPoolType::KERNEL,
								   (uint)program->userVirtual.resources.bitmap,
								   bitmapPages);
	}

	// 第三步，将未完成的子进程挂到 init 进程中
	for (auto pid : allPrograms) {
		PCB* child = getPCB(pid);
		if (child->parentPid == program->pid) {
			child->parentPid = 1; // 将子进程的父进程设置为 init 进程
		}
	}

	// 第四步，如果当前进程没有父进程，则将自己也挂到 init 进程中。
	if (program->parentPid == 0) {
		program->parentPid = 1; // 将当前进程的父进程设置为 init 进程
	}

	// 第五步，立即执行线程/进程调度。
	schedule();
}

int ProgramManager::wait(int* retval) {
	bool interrupt, flag;

	while (true) {
		interrupt = interruptManager.getInterruptStatus();
		interruptManager.disableInterrupt();
		auto it = allPrograms.find_if([&](int pid) {
			return getPCB(pid)->parentPid == this->running->pid;
		});

		if (it) // 找到一个可返回的子进程
		{
			auto child = getPCB(*it);
			if (child->status == ProgramStatus::DEAD) {
				int pid = child->pid;
				if (retval) {
					*retval = child->retValue;
				}
				releasePCB(child);
				interruptManager.setInterruptStatus(interrupt);
				return pid;
			}
			else {
				interruptManager.setInterruptStatus(interrupt);
				schedule(); // 存在子进程，但子进程的状态不是DEAD
			}
		}
		else {
			interruptManager.setInterruptStatus(interrupt);
			return -1;
		}
	}
}