// Kernel assert implementation

#include <chaos/assert.h>
#include <chaos/kprint.h>

void assert_handler(const char* file, u32 line) {
    kprint("Kernel assert!\n\t{s}: {d}\n", file, line);
    while (1);
}
