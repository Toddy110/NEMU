#include "cpu/exec/helper.h"

#include "cpu/cpu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

/*
 * Minimal interrupt/syscall support for this lab:
 * - int imm8 (0xCD)
 * - iret (0xCF)
 * - pusha/popa (0x60/0x61)
 * - cli/sti (0xFA/0xFB)
 * - hlt (0xF4)
 *
 * Notes:
 * - This code does NOT implement privilege level changes / task switch.
 * - Stack operations use sreg=3 (SS) per this project’s seg_translate mapping.
 */

static inline void push32(uint32_t val) {
  cpu.esp -= 4;
  swaddr_write(cpu.esp, 4, val, 3);
}

static inline uint32_t pop32(void) {
  uint32_t val = swaddr_read(cpu.esp, 4, 3);
  cpu.esp += 4;
  return val;
}

static void load_cs_cache(uint16_t selector) {
  /* Keep consistent with seg.c’s descriptor parsing logic. */
  cpu.cs.selector = selector;

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

  cpu.cs.base = base;
  cpu.cs.limit = limit;
}

make_helper(int_i_b) {
  uint8_t vec = instr_fetch(eip + 1, 1);
  uint32_t next_eip = eip + 2;

  /* Push EFLAGS, CS, EIP (32-bit slots). */
  push32(cpu.eflags.val);
  push32((uint32_t)cpu.cs.selector);
  push32(next_eip);

  /* Fetch IDT gate descriptor. */
  lnaddr_t gate = cpu.idtr.base + ((lnaddr_t)vec << 3);
  uint32_t low = lnaddr_read(gate, 4);
  uint32_t high = lnaddr_read(gate + 4, 4);

  uint16_t selector = (low >> 16) & 0xffff;
  uint32_t offset = (low & 0xffff) | (high & 0xffff0000);
  uint8_t type_attr = (high >> 8) & 0xff;

  Assert(type_attr & 0x80, "IDT gate not present: vec=%u idtr.base=0x%08x gate=0x%08x", vec, cpu.idtr.base, gate);

  load_cs_cache(selector);

  /* cpu_exec will add 2; compensate to land at offset. */
  cpu.eip = offset - 2;
  print_asm("int $0x%x", vec);
  return 2;
}

make_helper(iret) {
  uint32_t new_eip = pop32();
  uint16_t new_cs = (uint16_t)(pop32() & 0xffff);
  uint32_t new_eflags = pop32();

  cpu.eflags.val = new_eflags;
  load_cs_cache(new_cs);

  /* cpu_exec will add 1; compensate to land exactly. */
  cpu.eip = new_eip - 1;
  print_asm("iret");
  return 1;
}

make_helper(pusha) {
  uint32_t esp0 = cpu.esp;
  push32(cpu.eax);
  push32(cpu.ecx);
  push32(cpu.edx);
  push32(cpu.ebx);
  push32(esp0);
  push32(cpu.ebp);
  push32(cpu.esi);
  push32(cpu.edi);
  print_asm("pusha");
  return 1;
}

make_helper(popa) {
  cpu.edi = pop32();
  cpu.esi = pop32();
  cpu.ebp = pop32();
  (void)pop32(); /* skip ESP */
  cpu.ebx = pop32();
  cpu.edx = pop32();
  cpu.ecx = pop32();
  cpu.eax = pop32();
  print_asm("popa");
  return 1;
}

make_helper(cli) {
  cpu.eflags.IF = 0;
  print_asm("cli");
  return 1;
}

make_helper(sti) {
  cpu.eflags.IF = 1;
  print_asm("sti");
  return 1;
}

make_helper(hlt) {
  /* No hardware interrupt delivery in this lab setup; treat as a no-op. */
  print_asm("hlt");
  return 1;
}
