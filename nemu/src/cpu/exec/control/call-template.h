#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    uint32_t return_addr = cpu.eip + len + 1;
    
    cpu.esp -= DATA_BYTE;
    swaddr_write(cpu.esp, DATA_BYTE, return_addr);
    cpu.eip += (DATA_TYPE_S)op_src->val;
    
    print_asm("call: 0x%x", return_addr);
    return len + 1;
}

#include "cpu/exec/template-end.h"