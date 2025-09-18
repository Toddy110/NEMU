#include "cpu/exec/template-start.h"

#define make_jcc_helper(cc) \
    make_helper(concat4(j, cc, _, SUFFIX)){ \
        int len = concat(decode_si_, SUFFIX)(eip + 1); \
        swaddr_t target_addr = cpu.eip + op_src->val + 1 + len; \
        if (DATA_BYTE == 4) { \
            target_addr += 1; \
        } \
        print_asm(str(concat(j,cc)) " %x", target_addr); \
        if (concat(check_cc_, cc)()) { \
            cpu.eip += op_src->val; \
        } \
        return len + 1; \
    }

make_jcc_helper(e)
#include "cpu/exec/template-end.h"