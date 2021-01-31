// Top level network module

#include <chaos/network.h>
#include <chaos/netbuf.h>
#include <chaos/kprint.h>
#include <chaos/nic.h>

void network_start() {

    kprint("Starting networking\n");

    netbuf_init();
    nic_init();
}
