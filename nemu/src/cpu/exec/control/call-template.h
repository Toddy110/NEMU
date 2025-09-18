#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)){
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, eip + len);  // 存储返回地址（下一条指令的地址）
    
    cpu.eip = eip + (DATA_TYPE_S)op_src->val;  // 计算目标地址
    print_asm("call 0x%x", eip + (DATA_TYPE_S)op_src->val);
    
    return len + 1;
}


#include "cpu/exec/template-end.h"