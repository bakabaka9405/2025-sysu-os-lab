#pragma once
#include "os_type.hpp"
#include "flat_queue.hpp"
#include "program.hpp"

class SpinLock {
private:
	uint32 bolt;

public:
	SpinLock();
	void initialize();
	void lock();
	void unlock();
};

class Semaphore {
private:
	uint32 counter;
	FlatQueue<int, MAX_PROGRAM_AMOUNT> waiting;
	SpinLock semLock;

public:
	Semaphore();
	void initialize(uint32 counter);
	void P();
	void V();
};