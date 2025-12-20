#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

#define HW_MEM_SIZE (128 * 1024 * 1024)

extern uint8_t *hw_mem;

/* convert the hardware address in the test program to virtual address in NEMU */
#define hwa_to_va(p) ((void *)(hw_mem + (unsigned)p))
/* convert the virtual address in NEMU to hardware address in the test program */
#define va_to_hwa(p) ((hwaddr_t)((void *)p - (void *)hw_mem))

#define hw_rw(addr, type) *(type *)({\
	Assert(addr < HW_MEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
	hwa_to_va(addr); \
})

uint32_t swaddr_read(swaddr_t, size_t);
uint32_t lnaddr_read(lnaddr_t, size_t);
uint32_t hwaddr_read(hwaddr_t, size_t);
void swaddr_write(swaddr_t, size_t, uint32_t);
void lnaddr_write(lnaddr_t, size_t, uint32_t);
void hwaddr_write(hwaddr_t, size_t, uint32_t);

#endif

/* Page Directory Entry */
typedef union {
    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t page_write_through : 1;
        uint32_t page_cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t pad0 : 6;
        uint32_t page_frame : 20;
    };
    uint32_t val;
} PDE;

/* Page Table Entry */
typedef union {
    struct {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t page_write_through : 1;
        uint32_t page_cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pad0 : 1;
        uint32_t global : 1;
        uint32_t pad1 : 3;
        uint32_t page_frame : 20;
    };
    uint32_t val;
} PTE;
