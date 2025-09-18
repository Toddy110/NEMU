#include "cpu/exec/template-start.h"

#define instr ret

static void do_execute(){
    cpu.eip = swaddr_read(cpu.esp, 4) - 1;  // 从栈中读取返回地址，减1是因为cpu-exec.c会自动+1
    cpu.esp += 4;  // 恢复栈指针
    print_asm("ret");
}

make_helper(concat(ret_, SUFFIX)){
    do_execute();
    return 1;
}
#include "cpu/exec/template-end.h"