// Netbuf interface for the kernel TFTP stack

#ifndef NETBUF_H
#define NETBUF_H

#include <chaos/types.h>
#include <chaos/list.h>

// Must be aligned with at least 64-bytes
struct netbuf {
    u8 buf[1552];  // This must be the first node
    struct list_node node;

    // Allways pointing to the current protocol header start
    u8* ptr;
    u32 len;
};

void netbuf_init(void);

struct netbuf* alloc_netbuf(void);

void free_netbuf(struct netbuf* buf);

#endif
