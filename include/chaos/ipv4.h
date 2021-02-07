// IPv4 networking module for the kernel soft reboot

#ifndef IPv4_H
#define IPv4_H

#include <chaos/types.h>

i32 string_to_ipv4(const char* str, u32* ip_addr);

void ipv4_to_string(char* str, u32 addr);

#endif
