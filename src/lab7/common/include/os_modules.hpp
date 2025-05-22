#pragma once

#include "interrupt.hpp"
#include "program.hpp"
#include "stdio.hpp"

extern InterruptManager interruptManager;
extern ProgramManager programManager;
extern STDIO stdio;

#if __has_include("memory.hpp")
#include "memory.hpp"
extern MemoryManager memoryManager;
#endif