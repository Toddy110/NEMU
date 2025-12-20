#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

make_helper(lidt) {
    int len = decode_rm_l(eip + 1);
    cpu.idtr.limit = lnaddr_read(op_src->addr, 2);
    cpu.idtr.base = lnaddr_read(op_src->addr + 2, 4);
    print_asm("lidt %s", op_src->str);
    return len + 1;
}

make_helper(mov_sreg2rm) {
    int len = decode_r2rm_w(eip + 1);
    uint8_t sreg = op_src->reg;
    op_dest->val = cpu.sreg[sreg].selector;
    write_operand_w(op_dest, op_dest->val);
    print_asm("mov %%%s,%s", regsl[sreg], op_dest->str);
    return len + 1;
}
