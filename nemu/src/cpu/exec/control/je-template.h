#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
	if (cpu.eflags.ZF) {
		cpu.eip += (DATA_TYPE_S)op_src->val;
	}
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
