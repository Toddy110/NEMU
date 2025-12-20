#include "cpu/exec/helper.h"

make_helper(ret){
    uint32_t ra = swaddr_read(cpu.esp, 4, 3);
    if (ra == 0) {
        uint32_t w0 = swaddr_read(cpu.esp, 4, 3);
        uint32_t w1 = swaddr_read(cpu.esp + 4, 4, 3);
        uint32_t w2 = swaddr_read(cpu.esp + 8, 4, 3);
        uint32_t w3 = swaddr_read(cpu.esp + 12, 4, 3);
        printf("[NEMU] ret popped 0: cur_eip=0x%08x esp=0x%08x stack=[%08x %08x %08x %08x]\n",
               cpu.eip, cpu.esp, w0, w1, w2, w3);
    }
    cpu.eip = ra - 1;
    cpu.esp += 4; 
    print_asm("ret");
    return 1; 
}

make_helper(ret_i){
    uint16_t imm = instr_fetch(eip + 1, 2);
    cpu.eip = swaddr_read(cpu.esp, 4, 3) - 1 - 2; 
    cpu.esp += (4 + imm);  
    print_asm("ret 0x%x", imm);
    return 3;
}