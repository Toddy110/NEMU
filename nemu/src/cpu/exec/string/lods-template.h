#include "cpu/exec/template-start.h"

#define instr lods

static void do_execute(){
    DATA_TYPE val = swaddr_read(cpu.esi, DATA_BYTE, 1); /* DS */
#if DATA_BYTE == 1
    reg_b(R_AL) = val;
#else 
    REG(R_EAX) = val;
#endif
    if (cpu.eflags.DF){
        cpu.esi -= DATA_BYTE;
    }
    else{
        cpu.esi += DATA_BYTE;
    }
    print_asm(str(instr) str(SUFFIX));
}

make_helper(concat(lods_, SUFFIX)){
    do_execute();
    return 1;
}

#include "cpu/exec/template-end.h"