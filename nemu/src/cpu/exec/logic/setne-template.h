#include "cpu/exec/template-start.h"

#define instr setne

static void do_execute() {
    /* r/m8 destination already decoded by decode_rm_b */
    uint8_t v = (!cpu.eflags.ZF) ? 1 : 0;
    OPERAND_W(op_dest, v);
    print_asm(str(instr) " %s", op_dest->str);
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"