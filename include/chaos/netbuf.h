// Netbuf interface for the kernel network stack

#ifndef NETBUF_H
#define NETBUF_H

#include <chaos/types.h>
#include <chaos/list.h>

#define NETBUF_SIZE 1600

// Must be aligned with at least 64-bytes
struct netbuf {
    u8 buf[NETBUF_SIZE];

    // Allow to link netbuf's together. This also open for IP fragmenting
    struct list_node node;

    // Allways pointing to the current protocol header start
    u8* ptr;
    u32 len;
};

void netbuf_init();
struct netbuf* alloc_netbuf();
void free_netbuf(struct netbuf* buf);

#endif
