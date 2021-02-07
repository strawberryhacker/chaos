// ARP network module for the kernel soft reboot

#ifndef ARP_H
#define ARP_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

struct __attribute__((packed)) arp_header {
    u16 hardware_type;
    u16 protocol_type;
    u8  mac_len;
    u8  ip_len;
    u16 operation;
    u8  source_mac[6];
    u32 source_ip;
    u8  dest_mac[6];
    u32 dest_ip;
};

i32 arp_get_mac_addr(u32 ipv4_addr, u8* mac_addr);

void send_garp(u32 ipv4_addr);

void handle_arp(struct netbuf* buf);

#endif
