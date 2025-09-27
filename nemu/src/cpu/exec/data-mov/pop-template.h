#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute(){
    op_dest->val = swaddr_read(cpu.esp, 4);
    OPERAND_W(op_dest, op_dest->val);
    cpu.esp += 4;
    print_asm_template1();
}

make_helper(concat(pop_r_, SUFFIX)) {
    int reg_code = ops_decoded.opcode & 0x7;
    op_dest->type = OP_TYPE_REG;
    op_dest->reg = reg_code;
    do_execute();
    return 1;
}

make_helper(concat(pop_rm_, SUFFIX)){
    int len = concat(decode_rm_, SUFFIX)(eip + 1);
    do_execute();
    return len + 1;
}

#include "cpu/exec/template-end.h"