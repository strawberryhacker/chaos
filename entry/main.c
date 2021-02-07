// Chaos kernel

#include <chaos/types.h>
#include <chaos/kprint.h>
#include <chaos/panic.h>
#include <chaos/nic.h>
#include <chaos/cache.h>
#include <chaos/timer.h>
#include <chaos/boot_message.h>
#include <chaos/tftp.h>

// This is the memory padding between successive kernels
#define KERNEL_PADDING 1000

extern u32 linker_kernel_end;

char message[] = "Hello World";

void main() {

    kprint("\n\nStarting chaos kernel v2.0\n");
    //boot_start_timer();

    //tftp_init();
    //tftp_read_file(&linker_kernel_end + KERNEL_PADDING);

    while (1);
}
