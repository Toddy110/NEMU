#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    uint32_t return_addr = cpu.eip + len + 1;
    
    /* In our 32-bit environment, always push 4-byte return address
       to keep consistency with RET which pops 4 bytes. */
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, return_addr);
    cpu.eip += (DATA_TYPE_S)op_src->val;
    
    print_asm("call: 0x%x", return_addr);
    return len + 1;
}

#include "cpu/exec/template-end.h"