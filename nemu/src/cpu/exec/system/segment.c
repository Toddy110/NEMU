#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

void load_sreg(uint8_t sreg, uint16_t selector) {
    cpu.sreg[sreg].selector = selector;
    
    uint32_t idx = selector >> 3;
    // Assert((idx + 1) * 8 - 1 <= cpu.gdtr.limit, "Selector out of GDT limit");
    
    lnaddr_t addr = cpu.gdtr.base + (idx * 8);
    
    uint32_t low = lnaddr_read(addr, 4);
    uint32_t high = lnaddr_read(addr + 4, 4);
    
    uint32_t base_15_0 = low >> 16;
    uint32_t base_23_16 = (high >> 0) & 0xff;
    uint32_t base_31_24 = high >> 24;
    uint32_t base = (base_31_24 << 24) | (base_23_16 << 16) | base_15_0;
    
    uint32_t limit_15_0 = low & 0xffff;
    uint32_t limit_19_16 = (high >> 16) & 0xf;
    uint32_t limit = (limit_19_16 << 16) | limit_15_0;
    
    uint32_t granularity = (high >> 23) & 1;
    if (granularity) {
        limit = (limit << 12) | 0xfff;
    }
    
    cpu.sreg[sreg].base = base;
    cpu.sreg[sreg].limit = limit;
}

make_helper(lidt) {
    int len = decode_rm_l(eip + 1);
    cpu.idtr.limit = lnaddr_read(op_src->addr, 2);
    cpu.idtr.base = lnaddr_read(op_src->addr + 2, 4);
    print_asm("lidt %s", op_src->str);
    return len + 1;
}

make_helper(mov_rm2sreg) {
    int len = decode_rm2r_w(eip + 1);
    uint8_t sreg = op_dest->reg;
    uint16_t selector = op_src->val;
    load_sreg(sreg, selector);
    print_asm("mov %s,%%%s", op_src->str, regsl[sreg]);
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

make_helper(ljmp) {
    uint32_t offset = instr_fetch(eip + 1, 4);
    uint16_t selector = instr_fetch(eip + 5, 2);
    
    load_sreg(R_CS, selector);
    cpu.eip = offset;
    
    print_asm("ljmp /bin/bashx%x,/bin/bashx%x", selector, offset);
    return 0;
}
