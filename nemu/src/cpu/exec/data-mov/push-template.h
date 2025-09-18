#include "cpu/exec/template-start.h"

#define instr push
static void do_execute(){
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, op_src->val);
    print_asm_template1();
}

make_helper(concat(push_r_, SUFFIX)) {
    int reg_code = ops_decoded.opcode & 0x7;
    op_src->type = OP_TYPE_REG;
    op_src->reg = reg_code;
    op_src->val = REG(op_src->reg);
    do_execute();
    return 1;
}



#include "cpu/exec/template-end.h"