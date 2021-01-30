// Kernel panic implementation

#include <chaos/panic.h>
#include <chaos/kprint.h>

void panic(const char* message) {
    kprint("Kernel panic!\n\t{s}\n", message);
    while (1);
}
