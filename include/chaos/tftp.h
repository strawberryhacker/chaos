// TFTP network driver for the kernel soft reboot

#ifndef TFTP_H
#define TFTP_H

#include <chaos/types.h>

i32 tftp_read_file(void* dest, const char* file_name, const char* dest_ip,
    const char* source_ip);

#endif
