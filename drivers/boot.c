// Timestamped boot message implementation

#include <chaos/kprint.h>
#include <chaos/timer.h>
#include <chaos/panic.h>
#include <chaos/print_format.h>

#define BOOT_BUF_SIZE 1024

static char boot_message_buf[BOOT_BUF_SIZE];

void boot_start_timer() {
    const struct timer_iface* timer = get_timer();
    if (!timer) {
        panic("Wrong");
    }

    if (timer->init) {
        timer->init();
    }

    if (timer->restart) {
        timer->restart();
    }
}

void boot_message(const char* message, ...) {
    // Log the timestamp
    const struct timer_iface* timer = get_timer();
    if (timer) {
        if (timer->get_time) {
            u32 time = timer->get_time();
            kprint("[{3:u}.{0:3:u}] ", time / 1000, time % 1000);
        }
    }
    
    va_list arg;
    va_start(arg, message);
    u32 count = print_format_to_buf_arg(boot_message_buf, BOOT_BUF_SIZE, message, arg);
    va_end(arg);

    kprint_from_buf(boot_message_buf, count);
}
