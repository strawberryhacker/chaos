// Timer driver interface (kernel driver)

#ifndef TIMER_H
#define TIMER_H

#include <chaos/types.h>

struct timer_iface {
    void (*init)();
    void (*restart)();
    u32  (*get_time)();
};

// This should return NULL, or a new timer instance. The functions not implemeneted should
// be set to NULL
const struct timer_iface* get_timer();

#endif
