#include "tlb.h"
#include "cpu/cpu.h"
#include <stdlib.h>

TLBEntry tlb[64];

void tlb_init() {
    int i;
    for (i = 0; i < 64; i++) {
        tlb[i].valid = false;
    }
}

void tlb_flush() {
    int i;
    for (i = 0; i < 64; i++) {
        tlb[i].valid = false;
    }
}

int tlb_lookup(lnaddr_t addr, hwaddr_t *hwaddr) {
    uint32_t tag = (uint32_t)(addr >> 12);
    uint32_t offset = (uint32_t)(addr & 0xFFF);

    int i;
    for (i = 0; i < 64; i++) {
        TLBEntry *entry = &tlb[i];
        if (entry->valid && entry->tag == tag) {
            if (hwaddr != NULL) {
                *hwaddr = (hwaddr_t)((entry->page_frame << 12) + offset);
            }
            return 1;
        }
    }

    return 0;
}

void tlb_fill(lnaddr_t addr, hwaddr_t hwaddr) {
    uint32_t tag = (uint32_t)(addr >> 12);
    uint32_t page_frame = (uint32_t)(hwaddr >> 12);

    int idx = rand() % 64;
    tlb[idx].valid = 1;
    tlb[idx].tag = tag;
    tlb[idx].page_frame = page_frame;
}
