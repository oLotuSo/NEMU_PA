#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_num = is_mmio(addr);
  if (mmio_num != -1) {
    return mmio_read(addr, len, mmio_num);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int mmio_num = is_mmio(addr);
  if (mmio_num != -1) {
    mmio_write(addr, len, data, mmio_num);
  }
  else memcpy(guest_to_host(addr), &data, len);
}

#define PDX(va)     (((va) >> 22) & 0x3ff)
#define PTX(va)     (((va) >> 12) & 0x3ff)
#define OFFSET(va)     ((va) & 0xfff)
#define PTE_ADDR(pte)    ((pte) & 0xfffff000)

paddr_t page_translate(vaddr_t vaddr, bool flag){
  PDE pde;
  PTE pte;
  uint32_t pg, pt;
  pg = (PTE_ADDR(cpu.cr3.val)) | (PDX(vaddr) << 2);
  pde.val = paddr_read(pg, 4);
  Assert(pde.present, "pde.val: 0x%x", pde.val);
  pde.accessed = true;
  pt = (PTE_ADDR(pde.val)) | (PTX(vaddr) << 2);
  pte.val = paddr_read(pt, 4);
  Assert(pte.present, "pte.val: 0x%x, vaddr: 0x%x", pte.val, vaddr);
  pte.accessed = true;
  pte.dirty = flag ? 1 : pte.dirty;
  return PTE_ADDR(pte.val) | OFFSET(vaddr);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(cpu.cr0.paging) {
    if (((addr + len -1) & ~PAGE_MASK) != (addr & ~PAGE_MASK)) { //数据跨越虚拟页
        //读出这两个物理页中需要的字节, 然后拼接起来组成一个完整的数据返回
        uint32_t temp = 0;
        for (int i = 0; i < len; i++){
          paddr_t paddr = page_translate(addr + i, false);
          temp += paddr_read(paddr, 1) << (8 * i);

        }
        return temp;
    }
    else {
      paddr_t paddr = page_translate(addr, false);
      return paddr_read(paddr, len);
    }
  }
  else
    return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(cpu.cr0.paging) {
    if (((addr + len -1) & ~0xfff) != (addr & ~0xfff)) { 
      for (int i = 0; i < len; i++){
        paddr_t paddr = page_translate(addr + i, true);
        paddr_write(paddr, 1, data >> (8 * i));
      }
    }
    else {
      paddr_t paddr = page_translate(addr, true);
      paddr_write(paddr, len, data);
    }
  }
  else
    paddr_write(addr, len, data);
}
