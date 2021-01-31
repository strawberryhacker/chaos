// Kernel print functionality

#ifndef KPRINT_H
#define KPRINT_H

#include <chaos/types.h>
#include <stdarg.h>

void kprint(const char* message, ...);

void boot_message(const char* message, ...);

void kprint_from_buf(const char* buf, u32 size);

#endif
