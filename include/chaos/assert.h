// Kernel assert implementation

#ifndef ASSERT_H
#define ASSERT_H

#include <chaos/types.h>

#define assert(condition)                         \
    do {                                          \
        if (!(condition)) {                         \
            assert_handler(__FILE__, __LINE__);   \
        }                                         \
    } while (0)                                   \

void assert_handler(const char* file, u32 line);

#endif
