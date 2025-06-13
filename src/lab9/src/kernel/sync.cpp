#include "sync.hpp"
#include "asm_utils.hpp"
#include "program.hpp"

extern ProgramManager programManager;

SpinLock::SpinLock() {
	initialize();
}

void SpinLock::initialize() {
	bolt = 0;
}

void SpinLock::lock() {
	// uint32 key = 1;

	// do {
	// 	asm_atomic_exchange(&key, &bolt);
	// } while (key);
	asm_acquire_lock(&bolt);
}

void SpinLock::unlock() {
	// bolt = 0;
	asm_release_lock(&bolt);
}

Semaphore::Semaphore()
	: counter(0) {
	initialize(0);
}

void Semaphore::initialize(uint32 counter) {
	this->counter = counter;
	semLock.initialize();
}

void Semaphore::P() {
	PCB* cur = nullptr;

	while (true) {
		semLock.lock();
		if (counter > 0) {
			--counter;
			semLock.unlock();
			return;
		}

		cur = programManager.running;
		waiting.push(programManager.getPCBIndex(cur));
		cur->status = ProgramStatus::BLOCKED;

		semLock.unlock();
		programManager.schedule();
	}
}

void Semaphore::V() {
	semLock.lock();
	++counter;
	if (!waiting.empty()) {
		PCB* program = programManager.getPCB(waiting.front());
		waiting.pop();
		semLock.unlock();
		programManager.MESA_WakeUp(program);
	}
	else {
		semLock.unlock();
	}
}