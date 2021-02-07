// TFTP network driver for the kernel soft reboot

#ifndef TFTP_H
#define TFTP_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

void tftp_init();
i32 tftp_read_file(void* dest);

i32 string_to_ip(const char* str, u32* ip_addr);
void ip_to_string(char* str, u32 addr);

void mac_to_string(char* str, u8* mac, u8 lowercase);
i32 string_to_mac(const char* str, u8* mac);

#endif
