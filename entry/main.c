// Chaos kernel

#include <chaos/types.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>
#include <chaos/network.h>
#include <chaos/nic.h>
#include <chaos/cache.h>
#include <chaos/timer.h>
#include <chaos/boot.h>
#include <chaos/tftp.h>

extern u32 linker_kernel_end;

char message[] = "Hello World";

void main() {

    kprint("\n\nStarting chaos kernel v2.0\n");
    boot_start_timer();
    network_start();

    tftp_read_file(&linker_kernel_end, "sama5.bin", "192.168.10.10", "192.168.10.3");

    while (1);
}
