// Kernel print implementation

#include <chaos/kprint.h>
#include <chaos/print_format.h>

#define KPRINT_BUF_SIZE 1024

static char kprint_buf[KPRINT_BUF_SIZE];
static char boot_message_buf[KPRINT_BUF_SIZE];

void kprint(const char* message, ...) {
    va_list arg;
    va_start(arg, message);
    u32 count =
        print_format_to_buf_arg(kprint_buf, KPRINT_BUF_SIZE, message, arg);
    va_end(arg);

    kprint_from_buf(kprint_buf, count);
}

void boot_message(const char* message, ...) {
    // Log any timestamps?
    
    va_list arg;
    va_start(arg, message);
    u32 count =
        print_format_to_buf_arg(boot_message_buf, KPRINT_BUF_SIZE, message, arg);
    va_end(arg);

    kprint_from_buf(boot_message_buf, count);
}
