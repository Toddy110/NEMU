#include "cpu/exec/template-start.h"

#define instr call


make_helper(concat(call_i_, SUFFIX)){  //定义call指令跳转相对偏移量的操作
    int len = concat(decode_i_, SUFFIX)(eip + 1); //decode函数中解码立即数的操作获取偏移量长度
    
    // 计算返回地址（下一条指令的地址）
    swaddr_t return_addr = eip + len;
    
    // 将返回地址压入栈中
    reg_l(R_ESP) -= DATA_BYTE;  //esp-4操作，相当于压地址入栈的第一步，栈顶地址减4
    MEM_W(reg_l(R_ESP), return_addr);  //将返回地址写入栈顶
 
    // 计算跳转目标地址，考虑到cpu-exec.c会自动加len，所以这里要减去len
    cpu.eip = eip + (DATA_TYPE_S)op_src->val; //跳转到目标地址
    print_asm("call: 0x%x", eip + len + (DATA_TYPE_S)op_src->val);  //打印实际跳转地址
    return len + 1;  //返回指令长度
}

#include "cpu/exec/template-end.h"