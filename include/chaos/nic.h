// Core network driver interface

#ifndef NIC_H
#define NIC_H

#include <chaos/types.h>
#include <chaos/netbuf.h>

// Read and write raw data to the port

void nic_init(void);

struct netbuf* nic_recv(void);

void nic_send(struct netbuf* buf);

#endif
