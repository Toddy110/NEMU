#ifndef __MEMORY_CACHE_H__
#define __MEMORY_CACHE_H__
#include "common.h"

void init_cache(void);
uint32_t cache_hwaddr_read(hwaddr_t addr, size_t len);
void cache_hwaddr_write(hwaddr_t addr, size_t len, uint32_t data);

#define CACHE_b 6
#define CACHE_L1_e 3
#define CACHE_L1_s 7
#define CACHE_L2_e 4
#define CACHE_L2_s 12
#define CACHE_L1_CAP (64 * 1024)
#define CACHE_L2_CAP (4 * 1024 * 1024)
#define CACHE_B (1 << CACHE_b)
#define CACHE_L1_E (1 << CACHE_L1_e)
#define CACHE_L1_S (1 << CACHE_L1_s)
#define CACHE_L2_E (1 << CACHE_L2_e)
#define CACHE_L2_S (1 << CACHE_L2_s)
 
typedef struct{
    uint8_t data[CACHE_B];
    uint32_t tag;
    bool validVal;
} L1;

L1 cache_L1[CACHE_L1_S * CACHE_L1_E];
 
typedef struct{
    uint8_t data[CACHE_B];
    uint32_t tag;
    bool validVal;
    bool dirtyVal;
} L2;
 
L2 cache_L2[CACHE_L2_S * CACHE_L2_E];

#endif /* __MEMORY_CACHE_H__ */

#endif /* __MEMORY_CACHE_H__ */