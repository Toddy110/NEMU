#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "memory/memory.h"

uint32_t dram_read(hwaddr_t, size_t); /* still used indirectly during fills */
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) { return cache_hwaddr_read(addr, len); }
void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) { cache_hwaddr_write(addr, len, data); }

typedef struct {
    bool valid;
    uint32_t tag;
    uint32_t page_frame;
} TLBEntry;

TLBEntry tlb[64];

void init_tlb() {
    int i;
    for (i = 0; i < 64; i++) {
        tlb[i].valid = 0;
    }
}

hwaddr_t page_translate(lnaddr_t addr) {
    if (!cpu.cr0.PG) {
        return addr;
    }

    uint32_t tag = (addr >> 12);
    int i;
    for (i = 0; i < 64; i++) {
        if (tlb[i].valid && tlb[i].tag == tag) {
            return (tlb[i].page_frame << 12) + (addr & 0xfff);
        }
    }

    uint32_t dir = (addr >> 22) & 0x3ff;
    uint32_t page = (addr >> 12) & 0x3ff;
    uint32_t offset = addr & 0xfff;

    hwaddr_t pde_addr = (cpu.cr3.page_directory_base << 12) + (dir * 4);
    PDE pde;
    pde.val = hwaddr_read(pde_addr, 4);
    Assert(pde.present, "PDE not present at 0x%x for linear address 0x%x", pde_addr, addr);

    hwaddr_t pte_addr = (pde.page_frame << 12) + (page * 4);
    PTE pte;
    pte.val = hwaddr_read(pte_addr, 4);
    Assert(pte.present, "PTE not present at 0x%x for linear address 0x%x", pte_addr, addr);

    static int tlb_idx = 0;
    tlb[tlb_idx].valid = 1;
    tlb[tlb_idx].tag = tag;
    tlb[tlb_idx].page_frame = pte.page_frame;
    tlb_idx = (tlb_idx + 1) % 64;

    return (pte.page_frame << 12) + offset;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
    if (((addr & 0xfff) + len) > 0x1000) {
        /* Cross page boundary */
        uint32_t data = 0;
        int i; for (i = 0; i < len; i++) {
            hwaddr_t hwaddr = page_translate(addr + i);
            uint32_t b = hwaddr_read(hwaddr, 1);
            data |= (b << (i * 8));
        }
        return data;
    } else {
        hwaddr_t hwaddr = page_translate(addr);
        return hwaddr_read(hwaddr, len);
    }
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
    if (((addr & 0xfff) + len) > 0x1000) {
        /* Cross page boundary */
        int i; for (i = 0; i < len; i++) {
            hwaddr_t hwaddr = page_translate(addr + i);
            uint8_t b = (data >> (i * 8)) & 0xff;
            hwaddr_write(hwaddr, 1, b);
        }
    } else {
        hwaddr_t hwaddr = page_translate(addr);
        hwaddr_write(hwaddr, len, data);
    }
}

lnaddr_t segment_translate(swaddr_t addr, size_t len, uint8_t sreg) {
    if (cpu.cr0.PE) {
        Assert(sreg >= 0 && sreg < 6, "Invalid segment register %d", sreg);
        uint32_t base = cpu.sreg[sreg].base;
        uint32_t limit = cpu.sreg[sreg].limit;
        Assert(addr + len - 1 <= limit, "Segment limit exceeded at 0x%x (limit 0x%x)", addr, limit);
        return base + addr;
    }
    return addr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
        lnaddr_t lnaddr = segment_translate(addr, len, R_DS);
        return lnaddr_read(lnaddr, len);
}

uint32_t swaddr_read_instr(swaddr_t addr, size_t len) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
        lnaddr_t lnaddr = segment_translate(addr, len, R_CS);
        return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
        lnaddr_t lnaddr = segment_translate(addr, len, R_DS);
        lnaddr_write(lnaddr, len, data);
}
