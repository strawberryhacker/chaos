// Chaos kernel

#include <chaos/types.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>
#include <chaos/network.h>
#include <chaos/nic.h>
#include <chaos/cache.h>
#include <chaos/timer.h>
#include <chaos/boot.h>

void main() {

    kprint("\n\nStarting chaos kernel v1.0\n");
    boot_start_timer();
    network_start();

    while (1);
}
