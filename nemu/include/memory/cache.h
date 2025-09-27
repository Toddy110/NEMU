#ifndef __MEMORY_CACHE_H__
#define __MEMORY_CACHE_H__
#include "common.h"

void init_cache(void);
uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len);
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data);

#endif
