// Chaos kernel main function

#include <chaos/types.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>
#include <chaos/network.h>
#include <chaos/nic.h>
#include <chaos/cache.h>

void main() {

    kprint("\n\nStarting chaos kernel v1.0\n");

    network_start();

    while (1) {
        for (u32 i = 0; i < 500000; i++) {
            asm ("nop");
        }
        struct netbuf* buf = nic_recv();
        if (buf) {
            kprint("Got a packet => {d}\n", buf->len);
            for (u32 i = 0; i < buf->len; i++) {
                kprint("{0:2:x} ", buf->buf[i]);
            }
            kprint("\n\n");

            // Free the netbuffer
            free_netbuf(buf);
        }
    }

    while (1);
}