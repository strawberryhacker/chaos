// TFTP network driver for the kernel soft reboot

#ifndef TFTP_H
#define TFTP_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

#define MAC_TYPE_IPv4   0x0800
#define MAC_TYPE_ARP    0x0806

void copy_mac_addr(const u8* source, u8* dest);

void get_mac_addr(u8* mac);

i32 arp_get_mac_addr(u32 ipv4_addr, u8* mac_addr);

void send_gratuitous_arp(u32 ipv4_addr);

void handle_arp(struct netbuf* buf);

i32 string_to_ip(const char* str, u32* ip_addr);

void ip_to_string(char* str, u32 addr);

i32 tftp_read_file(void* dest, const char* file_name, const char* dest_ip,
    const char* source_ip);

void tftp_init();

void mac_to_string(char* str, u8* mac, u8 lowercase);

i32 string_to_mac(const char* str, u8* mac);

#endif
