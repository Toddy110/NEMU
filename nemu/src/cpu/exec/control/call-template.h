#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)){
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    swaddr_t ret = eip + 1 + len;
    cpu.esp -= DATA_BYTE;
    swaddr_write(cpu.esp, DATA_BYTE, ret);

    cpu.eip = ret + (DATA_TYPE_S)op_src->val;

    print_asm("call %x", cpu.eip);
    return 0;
}

make_helper(concat(call_rm_, SUFFIX)){
    int len = concat(decode_rm_, SUFFIX)(eip + 1);
    swaddr_t ret = eip + 1 + len;
    cpu.esp -= DATA_BYTE;
    swaddr_write(cpu.esp, DATA_BYTE, ret);

    cpu.eip = op_src->val;
    print_asm("call *%s", op_src->str);
    return 0;
    
}
#include "cpu/exec/template-end.h"