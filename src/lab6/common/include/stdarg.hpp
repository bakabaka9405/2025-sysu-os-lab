#pragma once
using va_list = char*;
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) ((ap) = (va_list) & (v) + _INTSIZEOF(v))
#define va_arg(ap, type) (*(type*)(((ap) += _INTSIZEOF(type)) - _INTSIZEOF(type)))
#define va_end(ap) ((ap) = (va_list)0)
