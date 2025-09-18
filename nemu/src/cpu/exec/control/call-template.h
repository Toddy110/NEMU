#include "cpu/exec/template-start.h"

#define instr call


make_helper(concat(call_i_, SUFFIX)){  //定义call指令跳转相对偏移量的操作
    int len = concat(decode_i_, SUFFIX)(eip + 1); //decode函数中解码立即数的操作获取偏移量长度
    reg_l(R_ESP) -= DATA_BYTE;  //esp-4操作，相当于压地址入栈的第一步，栈顶地址减4
    MEM_W(reg_l(R_ESP), cpu.eip+len+1);  //将下一步的指令地址写入栈顶
 
    cpu.eip += (DATA_TYPE_S)op_src->val; //加上立即数偏移量，实现跳转操作
    print_asm("call: 0x%x", cpu.eip + len + 1);  //打印调试信息，查看跳转的具体地址
    return len + 1;  //返回指令长度
}

#include "cpu/exec/template-end.h"