#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

#include "cpu/cpu.h"
#include "memory/memory.h"

/*
 * 0F 01 /2: lgdt m16&32
 * Read 6 bytes from memory: limit(16) then base(32).
 */
make_helper(lgdt) {
  int len = decode_rm_l(eip + 1);

  /* operand is a memory location holding a 6-byte pseudo-descriptor */
  cpu.gdtr.limit = lnaddr_read(op_src->addr, 2);
  cpu.gdtr.base = lnaddr_read(op_src->addr + 2, 4);

  print_asm("lgdt %s", op_src->str);
  return len + 1;
}

/*
 * 0F 20 /r: mov r32, CRn
 * We only need CR0 for this lab (reg field == 0).
 */
make_helper(mov_cr2r) {
  ModR_M m;
  m.val = instr_fetch(eip + 1, 1);
  int cr = m.reg;
  int r = m.R_M;

  if (cr == 0) {
    reg_l(r) = cpu.cr0.val;
  } else if (cr == 3) {
    /* Keep compatibility with existing implementation. */
    reg_l(r) = cpu.cr3.val;
  } else {
    /* Unsupported CRn in this lab; ignore to avoid crashing release builds. */
  }

  print_asm("movl %%cr%d,%%%s", cr, regsl[r]);
  return 2;
}

/*
 * 0F 22 /r: mov CRn, r32
 * We only need CR0 for this lab (reg field == 0).
 */
make_helper(mov_r2cr) {
  ModR_M m;
  m.val = instr_fetch(eip + 1, 1);
  int cr = m.reg;
  int r = m.R_M;

  if (cr == 0) {
    cpu.cr0.val = reg_l(r);
  } else if (cr == 3) {
    /* Keep compatibility with existing implementation. */
    cpu.cr3.val = reg_l(r);
    init_tlb();
  } else {
    /* Unsupported CRn in this lab; ignore to avoid crashing release builds. */
  }

  print_asm("movl %%%s,%%cr%d", regsl[r], cr);
  return 2;
}
