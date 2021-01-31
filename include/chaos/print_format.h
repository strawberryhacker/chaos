// Formatted print to sized buffer implementation

#ifndef PRINT_FORMAT_H
#define PRINT_FORMAT_H

#include <chaos/types.h>
#include <stdarg.h>

u32 print_format_to_buf_arg(char* buf, u32 len, const char* str, va_list arg);

u32 print_format_to_buf(char* buf, u32 len, const char* str, ...);

#endif
