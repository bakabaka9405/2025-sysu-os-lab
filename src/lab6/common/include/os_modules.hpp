#pragma once

#if __has_include("interrupt.hpp")
#include "interrupt.hpp"
extern InterruptManager interruptManager;
#endif

#if __has_include("program.hpp")
#include "program.hpp"
extern ProgramManager programManager;
#endif

#if __has_include("stdio.hpp")
#include "stdio.hpp"
extern STDIO stdio;
#endif