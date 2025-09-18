#include "cpu/exec/template-start.h"

#define instr call

static void do_execute() {
    swaddr_t return_addr = cpu.eip + 1 + DATA_BYTE;
    
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, return_addr);
    
    cpu.eip += (DATA_TYPE_S)op_src->val - 1 - DATA_BYTE;
    
    print_asm("call 0x%x", cpu.eip + 1 + DATA_BYTE);
}

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    do_execute();
    return len + 1;
}

#include "cpu/exec/template-end.h"