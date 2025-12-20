#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "memory/memory.h"
#include "memory/tlb.h"

uint32_t dram_read(hwaddr_t, size_t); /* still used indirectly during fills */
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) { return cache_hwaddr_read(addr, len); }
void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) { cache_hwaddr_write(addr, len, data); }

/* Backward-compatible entry point used by existing code (e.g., CR3 updates). */
void init_tlb() { tlb_init(); }

hwaddr_t page_translate(lnaddr_t addr) {
    if (!cpu.cr0.PG) {
        return addr;
    }

    hwaddr_t hwaddr;
    if (tlb_lookup(addr, &hwaddr)) {
        return hwaddr;
    }

    uint32_t dir = (addr >> 22) & 0x3FF;
    uint32_t page = (addr >> 12) & 0x3FF;
    uint32_t offset = addr & 0xFFF;

    uint32_t pde_addr = cpu.cr3 + dir * 4;
    uint32_t pde = hwaddr_read(pde_addr, 4);
        Assert(pde & 0x1, "PDE not present: lnaddr=0x%08x cr3=0x%08x pde_addr=0x%08x pde=0x%08x dir=%u",
            addr, cpu.cr3, pde_addr, pde, dir);

    uint32_t pte_addr = (pde & 0xFFFFF000) + page * 4;
    uint32_t pte = hwaddr_read(pte_addr, 4);
        Assert(pte & 0x1, "PTE not present: lnaddr=0x%08x cr3=0x%08x pte_addr=0x%08x pte=0x%08x dir=%u page=%u",
            addr, cpu.cr3, pte_addr, pte, dir, page);

    hwaddr = (pte & 0xFFFFF000) + offset;
    tlb_fill(addr, hwaddr);
    return hwaddr;
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
    if (((addr & 0xfff) + len) > 0x1000) {
        /* Cross page boundary: terminate for now (can be optimized later). */
        assert(0);
        return 0;
    } else {
        /* Enable paging only when PE=1 and PG=1. */
        if ((cpu.cr0.val & 0x1) && (cpu.cr0.val & CR0_PG)) {
            hwaddr_t hwaddr = page_translate(addr);
            return hwaddr_read(hwaddr, len);
        } else {
            return hwaddr_read(addr, len);
        }
    }
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
    if (((addr & 0xfff) + len) > 0x1000) {
        /* Cross page boundary: terminate for now (can be optimized later). */
        assert(0);
    } else {
        /* Enable paging only when PE=1 and PG=1. */
        if ((cpu.cr0.val & 0x1) && (cpu.cr0.val & CR0_PG)) {
            hwaddr_t hwaddr = page_translate(addr);
            hwaddr_write(hwaddr, len, data);
        } else {
            hwaddr_write(addr, len, data);
        }
    }
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
    /*
     * Real mode: linear address == logical address.
     * Protected mode: linear address == segment.base + offset.
     */
    if ((cpu.cr0.val & 0x1) == 0) {
        return addr;
    }

    SegmentReg *seg = NULL;
    switch (sreg) {
        case 0: seg = &cpu.cs; break;
        case 1: seg = &cpu.ds; break;
        case 2: seg = &cpu.es; break;
        case 3: seg = &cpu.ss; break;
        default:
            Assert(0, "Invalid segment selector id %d (expected 0=CS,1=DS,2=ES,3=SS)", sreg);
    }

    /*
     * Bound check: ensure [addr, addr+len-1] is within segment limit.
     * Use 64-bit arithmetic to avoid overflow.
     */
    uint64_t end = (uint64_t)addr + (uint64_t)len - 1;
    Assert(end <= seg->limit, "Segment limit exceeded: offset=0x%x len=%zu limit=0x%x", addr, len, seg->limit);

    return seg->base + addr;
}

/* Compatibility with existing code/header naming. */
lnaddr_t segment_translate(swaddr_t addr, size_t len, uint8_t sreg) {
    return seg_translate(addr, len, sreg);
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
    lnaddr_t lnaddr = seg_translate(addr, len, sreg);
    return lnaddr_read(lnaddr, len);
}

uint32_t swaddr_read_instr(swaddr_t addr, size_t len) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
    /* instruction fetch is bound to CS=0 */
    return swaddr_read(addr, len, 0);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
        assert(len == 1 || len == 2 || len == 4);
#endif
    lnaddr_t lnaddr = seg_translate(addr, len, sreg);
    lnaddr_write(lnaddr, len, data);
}
