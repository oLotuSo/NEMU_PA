#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  vaddr_t gate_addr = cpu.idtr.base + 8 * NO;
  uint32_t t1, t2;
  t1 = vaddr_read(gate_addr, 4) & 0xffff;
  t2 = vaddr_read(gate_addr + 4, 4) & 0xffff0000;

  if (cpu.idtr.limit < 0) assert(0);

  rtl_push(&cpu.eflags.val);
  t0 = cpu.cs; // 这里把16位的cs扩展到32位
  rtl_push(&t0);
  rtl_push(&ret_addr);

  decoding.jmp_eip = t1 | t2;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
