#ifndef __CPU_EXEC_SYSTEM_INTR_H__
#define __CPU_EXEC_SYSTEM_INTR_H__

#include "cpu/helper.h"

make_helper(int_i_b);
make_helper(iret);

make_helper(pusha);
make_helper(popa);

make_helper(cli);
make_helper(sti);

make_helper(hlt);

#endif
