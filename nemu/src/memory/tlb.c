#include "tlb.h"
#include "cpu/cpu.h"

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
        if (tlb[i].valid && tlb[i].tag == tag) {
            if (hwaddr != NULL) {
                *hwaddr = (hwaddr_t)((tlb[i].page_frame << 12) | offset);
            }
            return 1;
        }
    }
    return 0;
}

void tlb_fill(lnaddr_t addr, hwaddr_t hwaddr) {
    static int tlb_idx = 0;

    tlb[tlb_idx].valid = true;
    tlb[tlb_idx].tag = (uint32_t)(addr >> 12);
    tlb[tlb_idx].page_frame = (uint32_t)(hwaddr >> 12);

    tlb_idx = (tlb_idx + 1) % 64;
}
