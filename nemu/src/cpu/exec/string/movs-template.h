#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_, SUFFIX)) {
	DATA_TYPE val = swaddr_read(cpu.esi, DATA_BYTE, 1); /* DS */
	swaddr_write(cpu.edi, DATA_BYTE, val, 2); /* ES */
	cpu.esi += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);
	cpu.edi += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);

	print_asm("movs" str(SUFFIX) " %%ds:(%%esi),%%es:(%%edi)");
	return 1;
}

#include "cpu/exec/template-end.h"
