#ifndef __NEMU_MEMORY_TLB_H__
#define __NEMU_MEMORY_TLB_H__

#include "common.h"
#include "memory/memory.h"  // for lnaddr_t/hwaddr_t

typedef struct {
    bool valid;
    uint32_t tag;
    uint32_t page_frame;
} TLBEntry;

extern TLBEntry tlb[64];

void tlb_init();
int tlb_lookup(lnaddr_t addr, hwaddr_t *hwaddr);
void tlb_fill(lnaddr_t addr, hwaddr_t hwaddr);
void tlb_flush();

#endif
