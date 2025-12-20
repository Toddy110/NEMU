#include "cpu/exec/helper.h"
#include "cpu/decode/modrm.h"

#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/tlb.h"

static void load_sreg(uint8_t sreg, uint16_t selector) {
  cpu.sreg[sreg].selector = selector;

  uint32_t idx = selector >> 3;
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
    reg_l(r) = cpu.cr3;
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
    cpu.cr3 = reg_l(r) & 0xfffff000u;
    tlb_flush();
    init_tlb();
  } else {
    /* Unsupported CRn in this lab; ignore to avoid crashing release builds. */
  }

  print_asm("movl %%%s,%%cr%d", regsl[r], cr);
  return 2;
}

/*
 * 8E /r: mov Sreg, r/m16
 * Only DS/ES/SS are required here; IA-32 forbids loading CS via MOV.
 */
make_helper(mov_rm2sreg) {
  int len = decode_rm2r_w(eip + 1);
  uint8_t sreg = op_dest->reg;
  uint16_t selector = op_src->val;

  Assert(sreg == R_DS || sreg == R_ES || sreg == R_SS,
         "Only DS/ES/SS are supported for mov to sreg (got %d)", sreg);

  load_sreg(sreg, selector);
  print_asm("mov %s,%%%s", op_src->str, regsl[sreg]);
  return len + 1;
}

/*
 * EA cd: ljmp ptr16:32
 * Far jump (offset32 + selector16). Updates CS via descriptor cache.
 */
make_helper(ljmp) {
  uint32_t offset;
  uint16_t selector;
  int len;

  /* In 16-bit operand-size mode, ljmp uses ptr16:16 (offset16 + selector16).
   * Otherwise, it uses ptr16:32 (offset32 + selector16).
   */
  if (ops_decoded.is_operand_size_16) {
    offset = instr_fetch(eip + 1, 2);
    selector = instr_fetch(eip + 3, 2);
    len = 5;
  } else {
    offset = instr_fetch(eip + 1, 4);
    selector = instr_fetch(eip + 5, 2);
    len = 7;
  }

  load_sreg(R_CS, selector);
  /* cpu_exec will add returned length; compensate to land at offset. */
  cpu.eip = offset - (uint32_t)len;

  print_asm("ljmp $0x%x,$0x%x", selector, offset);
  return len;
}
