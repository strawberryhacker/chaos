// NIC driver for SAMA5D2 chips (kernel driver)

#ifndef NIC_H
#define NIC_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

void nic_init();
struct netbuf* nic_receive();
void nic_send(struct netbuf* buf);

#endif
