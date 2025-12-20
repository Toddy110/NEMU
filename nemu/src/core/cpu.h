#ifndef __NEMU_SRC_CORE_CPU_H__
#define __NEMU_SRC_CORE_CPU_H__

/*
 * Compatibility header.
 *
 * Some lab specs refer to "nemu/src/core/cpu.h". In this codebase, the actual
 * CPU_state definition lives in "nemu/include/cpu/reg.h" (and is typically
 * included via the normal include path).
 *
 * - CPU_state already contains:
 *     uint32_t cr3;   // page directory base physical address, low 12 bits are 0
 * - CR0 paging bit macro:
 *     CR0_PG (1u << 31)
 */

#include "cpu/reg.h"

#ifndef CR0_PG
#define CR0_PG (1u << 31)
#endif

#endif
