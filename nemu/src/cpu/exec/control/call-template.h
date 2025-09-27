#include "cpu/exec/template-start.h"

#define instr call

/* Unified implementation: always push 4-byte return address */
make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(cpu.eip + 1);
    uint32_t return_addr = cpu.eip + len + 1;
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, return_addr);
    cpu.eip += (DATA_TYPE_S)op_src->val;
    print_asm("call %x", cpu.eip);
    return len + 1;
}

make_helper(concat(call_rm_, SUFFIX)) {
    int len = concat(decode_rm_, SUFFIX)(cpu.eip + 1);
    uint32_t return_addr = cpu.eip + len + 1;
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, return_addr);
    cpu.eip = op_src->val;
    print_asm("call *%s", op_src->str);
    return len + 1;
}

#include "cpu/exec/template-end.h"