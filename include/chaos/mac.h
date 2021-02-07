// MAC network module for the kernel soft reboot

#ifndef MAC_H
#define MAC_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

#define MAC_TYPE_IPv4   0x0800
#define MAC_TYPE_ARP    0x0806

void copy_mac_addr(const u8* source, u8* dest);

void get_mac_addr(u8* mac);

void mac_send_to(struct netbuf* buf, u8* dest_mac, u16 type);

#endif
