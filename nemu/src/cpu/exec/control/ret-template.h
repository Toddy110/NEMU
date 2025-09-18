#include "cpu/exec/template-start.h"

#define instr ret

static void do_execute(){
    cpu.eip = swaddr_read(cpu.eip, 4) - 1 - DATA_BYTE;
    cpu.esp += 4;
    print_asm("ret");
}

make_helper(concat(ret_, SUFFIX)){
    do_execute();
    return 1;
}
#include "cpu/exec/template-end.h"