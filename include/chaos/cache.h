// Cache interface for L1 data cache & instruction cache

#ifndef CACHE_H
#define CACHE_H

#include <chaos/types.h>

extern void icache_enable();
extern void icache_disable();
extern void icache_invalidate();

extern void dcache_enable();
extern void dcache_disable();
extern void dcache_clean();
extern void dcache_clean_virt_range(u32 start, u32 end);
extern void dcache_invalidate();
extern void dcache_invalidate_virt_range(u32 start, u32 end);
extern void dcache_clean_invalidate();
extern void dcache_clean_invalidate_virt_range(u32 start, u32 end);

#endif