// Network interface for the kernel network stack

#include <chaos/network.h>
#include <chaos/netbuf.h>
#include <chaos/kprint.h>
#include <chaos/nic.h>
#include <chaos/kprint.h>

void network_start() {
    boot_message("Starting networking\n");
    
    netbuf_init();
    nic_init();

    boot_message("Networking ready\n");
}
