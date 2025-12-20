#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, op_src->val, 3);
    print_asm_template1();
}

make_helper(concat(push_i_, SUFFIX)){
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    if (DATA_BYTE == 1){
        op_src->val = (int32_t)(int8_t)op_src->val;
    } 
    do_execute();
    return len + 1;
}


#if DATA_BYTE == 4 || DATA_BYTE == 2
make_helper(concat(push_r_, SUFFIX)) {
    int reg_code = ops_decoded.opcode & 0x7;
    op_src->type = OP_TYPE_REG;
    op_src->reg = reg_code;
    op_src->val = REG(op_src->reg);
    do_execute();
    return 1;
}

make_helper(concat(push_rm_, SUFFIX)){
    int len = concat(decode_rm_, SUFFIX)(eip + 1);
    do_execute();
    return len + 1;
}
#endif

#include "cpu/exec/template-end.h"