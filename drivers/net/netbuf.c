// Netbuf interface for the kernel TFTP stack

#include <chaos/netbuf.h>
#include <chaos/assert.h>
#include <chaos/kprint.h>
#include <stdalign.h>

// Set this to the number of packets to buffer
#define NIC_MAX_BUF 256

// Pool of network buffers
static struct list_node netbuf_pool;

// Allocate the netbuffers
static alignas(32) struct netbuf buffers[NIC_MAX_BUF];

// Initializes the netbuffers
void netbuf_init() {
    // Initialize the netbuffer pool
    list_init(&netbuf_pool);

    for (u32 i = 0; i < NIC_MAX_BUF; i++) {
        list_push_front(&buffers[i].node, &netbuf_pool);
    }

    kprint("Initialized {d} netbuffers\n", NIC_MAX_BUF);
}

struct netbuf* alloc_netbuf() {
    // Get the first free netbuf list node
    struct list_node* node = list_pop_front(&netbuf_pool);
    assert(node);

    // Convert the list node to a netbuf and return it
    return list_get_struct(node, struct netbuf, node);
}

void free_netbuf(struct netbuf* buf) {
    // Get the first free netbuf list node
    list_push_back(&buf->node, &netbuf_pool);
}
