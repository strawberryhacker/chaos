// Timestamped boot message implementation

#ifndef BOOT_H
#define BOOT_H

#include <chaos/types.h>
#include <stdarg.h>

void boot_start_timer();

void boot_message(const char* message, ...);

#endif