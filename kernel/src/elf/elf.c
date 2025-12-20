#include "common.h"
#include "memory.h"
#include <string.h>
#include <elf.h>

#define ELF_OFFSET_IN_DISK 0

#ifdef HAS_DEVICE
void ide_read(uint8_t *, uint32_t, uint32_t);
#else
void ramdisk_read(uint8_t *, uint32_t, uint32_t);
#endif

#define STACK_SIZE (1 << 20)

void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader() {
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph = NULL;

	uint8_t buf[4096];

#ifdef HAS_DEVICE
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
#else
	ramdisk_read(buf, ELF_OFFSET_IN_DISK, 4096);
#endif

	elf = (void*)buf;

	/* TODO: fix the magic number with the correct one */
	const uint32_t elf_magic = 0x464c457f;
	uint32_t *p_magic = (void *)buf;
	nemu_assert(*p_magic == elf_magic);

	/* Load each program segment */
	ph = (void *)buf + elf->e_phoff;
	Elf32_Phdr *eph;

#ifdef IA32_PAGE
	/* Switch to user page directory early so that accesses to user virtual
	 * addresses go through the page tables we are going to build via mm_malloc().
	 */
	write_cr3(get_ucr3());
#endif
	
	for(eph = ph + elf->e_phnum; ph < eph; ph++) {
		/* Scan the program header table, load each segment into memory */
		if(ph->p_type == PT_LOAD) {
			uint32_t addr = ph->p_vaddr;

#ifdef IA32_PAGE
			/* Allocate physical memory for [addr, addr + MemSiz). This should also
			 * fill the page directory/page tables for the user address space.
			 */
			mm_malloc(addr, ph->p_memsz);
			/* Flush TLB to make sure the new mappings take effect immediately. */
			write_cr3(get_ucr3());
#endif
			/* TODO: read the content of the segment from the ELF file 
			 * to the memory region [VirtAddr, VirtAddr + FileSiz)
			 */
			#ifdef HAS_DEVICE
			ide_read((void *)addr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);
			#else
			ramdisk_read((void *)addr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);
			#endif

			 
			/* TODO: zero the memory region 
			 * [VirtAddr + FileSiz, VirtAddr + MemSiz)
			 */
			memset((void *)(addr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);

#ifdef IA32_PAGE
			/* Record the program break for future use. */
			extern uint32_t cur_brk, max_brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if(cur_brk < new_brk) { max_brk = cur_brk = new_brk; }
#endif
		}
	}

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
	/* Allocate user process stack: stack top is 0xc0000000, size is 1MB. */
	mm_malloc(0xc0000000 - (1024 * 1024), 1024 * 1024);
	write_cr3(get_ucr3());

	/*
	 * Some uclibc routines in this lab may execute `call 0` as a stub.
	 * Map the first page and place a single `ret` instruction there so the
	 * call returns immediately instead of faulting on an unmapped null page.
	 */
	mm_malloc(0x0, 4096);
	write_cr3(get_ucr3());
	{
		volatile uint8_t *tramp = (volatile uint8_t *)0x0;
		tramp[0] = 0xC3; /* ret */
	}

#ifdef HAS_DEVICE
	create_video_mapping();
#endif

	write_cr3(get_ucr3());
#endif

	return entry;
}
