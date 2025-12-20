#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)

/* Use the function to get the start address of user page directory. */
PDE* get_updir();

static PTE vptable[1024] align_to_page;

void create_video_mapping() {
        /* TODO: create an identical mapping from virtual memory area 
         * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area 
         * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
         * some page tables to create this mapping.
         */
        PDE *updir = get_updir();
        PDE *pde = &updir[0];
        PTE *pt;

        if (pde->present) {
            pt = (PTE *)pa_to_va(pde->page_frame << 12);
        } else {
            pt = vptable;
            pde->val = make_pde(va_to_pa(vptable));
        }

        uint32_t start_page = VMEM_ADDR >> 12;
        uint32_t end_page = (VMEM_ADDR + SCR_SIZE + 4095) >> 12;

        uint32_t i; for (i = start_page; i < end_page; i++) {
            pt[i].val = make_pte(i << 12);
        }
}

void video_mapping_write_test() {
        int i;
        uint32_t *buf = (void *)VMEM_ADDR;
        for(i = 0; i < SCR_SIZE / 4; i ++) {
                buf[i] = i;
        }
}

void video_mapping_read_test() {
        int i;
        uint32_t *buf = (void *)VMEM_ADDR;
        for(i = 0; i < SCR_SIZE / 4; i ++) {
                assert(buf[i] == i);
        }
}

void video_mapping_clear() {
        memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}
