#pragma once
#include "rr.hpp"

#define ProgramManager RRProgramManager

void initializePCB();

PCB* getPCB(int pid);

int getPCBIndex(PCB* program);

// 分配一个PCB
PCB* allocatePCB();
// 归还一个PCB
// program：待释放的PCB
void releasePCB(PCB* program);

void program_exit();
