#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
    int instr_len = 1 + DATA_BYTE;
    swaddr_t return_addr = cpu.eip + instr_len;
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, return_addr);
    cpu.eip = cpu.eip += (DATA_TYPE_S)op_src->val - instr_len;
    print_asm("call 0x%x", return_addr + (DATA_TYPE_S)op_src->val);
}

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    do_execute();
    return len + 1;
}

#include "cpu/exec/template-end.h"