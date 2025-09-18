#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)){
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, eip + len);  // 保存下一条指令的地址作为返回地址
    
    cpu.eip = eip + len + (DATA_TYPE_S)op_src->val;  // 跳转地址 = 当前指令地址 + 指令长度 + 偏移量
    print_asm("call %x", cpu.eip);
    
    return len + 1;
}


#include "cpu/exec/template-end.h"