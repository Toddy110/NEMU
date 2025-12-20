#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

make_helper(mov_r2cr) {
    ModR_M m;
    m.val = instr_fetch(eip + 1, 1);
    int reg = m.reg;
    int rm = m.R_M;

    if (reg == 0) {
        cpu.cr0.val = reg_l(rm);
    } else if (reg == 3) {
        cpu.cr3.val = reg_l(rm);
    } else {
        /* Assert(0, "Move to invalid control register CR%d", reg); */
    }

    print_asm("movl %%%s,%%cr%d", regsl[rm], reg);
    return 2;
}

make_helper(mov_cr2r) {
    ModR_M m;
    m.val = instr_fetch(eip + 1, 1);
    int reg = m.reg;
    int rm = m.R_M;

    if (reg == 0) {
        reg_l(rm) = cpu.cr0.val;
    } else if (reg == 3) {
        reg_l(rm) = cpu.cr3.val;
    } else {
        /* Assert(0, "Move from invalid control register CR%d", reg); */
    }

    print_asm("movl %%cr%d,%%%s", reg, regsl[rm]);
    return 2;
}
