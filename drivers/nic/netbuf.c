// Netbuf interface for the kernel network stack

#include <chaos/netbuf.h>
#include <chaos/assert.h>
#include <chaos/kprint.h>
#include <stdalign.h>

// This is the maximum possible header size. The netbuf allocator will reserve some space
// in the beginning of the buffer. This allows protocol layers to insert required
// network headers
#define MAX_HEADER_SIZE 134

// This indicates how many packets can be stored in the system at any time. For normal 
// TFTP / UDP / IP this number can be lower than 256
#define NIC_MAX_BUF 256
static alignas(32) struct netbuf buffers[NIC_MAX_BUF];

// This list keeps track of all unused netbuffers
static struct list_node netbuf_pool;

// Initializes the netbuffers
void netbuf_init() {
    list_init(&netbuf_pool);

    // Insert all the buffers into the list
    for (u32 i = 0; i < NIC_MAX_BUF; i++) {
        list_push_front(&buffers[i].node, &netbuf_pool);
    }
}

struct netbuf* alloc_netbuf() {
    // Get the first free netbuf list node
    struct list_node* node = list_pop_front(&netbuf_pool);
    assert(node);

    // Convert the list node to a netbuf and return it
    struct netbuf* buf = list_get_struct(node, struct netbuf, node);
    buf->ptr = buf->buf + MAX_HEADER_SIZE;
    return buf;
}

void free_netbuf(struct netbuf* buf) {
    // Get the first free netbuf list node
    list_push_back(&buf->node, &netbuf_pool);
}
