#ifndef __MEMORY_CACHE_H__
#define __MEMORY_CACHE_H__
#include "common.h"

void init_cache(void);
uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len);
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data);

#define CACHE_b 6
#define CACHE_e 3
#define CACHE_s 7
#define CACHE_CAP 65536
#define CACHE_B (1 << CACHE_b)
#define CACHE_E (1 << CACHE_e)
#define CACHE_S (1 << CACHE_s)

typedef struct {
  uint8_t data[CACHE_B];
  uint32_t tag;
  uint8_t valid;
} CacheBlock;
CacheBlock cache_L1[CACHE_S * CACHE_E];
#endif
