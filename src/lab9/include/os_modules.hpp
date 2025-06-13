#pragma once

#include "interrupt.hpp"
#include "program.hpp"
#include "stdio.hpp"
#include "memory.hpp"
#include "tss.hpp"
#include "syscall.hpp"

extern InterruptManager interruptManager;
extern ProgramManager programManager;
extern STDIO stdio;
extern MemoryManager memoryManager;
extern SystemService systemService;
extern TSS tss;