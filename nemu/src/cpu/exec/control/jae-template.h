#include "cpu/exec/template-start.h"

#define instr jae

static void do_execute() {
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE + cpu.eip + (DATA_TYPE_S)op_src->val);
	if (!cpu.eflags.CF) {
		cpu.eip += (DATA_TYPE_S)op_src->val;
	}
}


make_instr_helper(i)
#if DATA_BYTE == 4
make_helper(jae_rm_l) {
	int len = decode_rm_l(eip + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}
#endif

#include "cpu/exec/template-end.h"