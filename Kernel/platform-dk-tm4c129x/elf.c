#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <elf.h>

int platform_relocate_rel(Elf32_Rel *rel, uaddr_t section_base)
{
  uaddr_t addr;
  uaddr_t value;

  switch (ELF32_R_TYPE(rel->r_info))
  {
  case R_ARM_RELATIVE:
    addr = section_base + (rel->r_offset);
    value = ugetp((void *)(addr)) + section_base;
    uputp(value, ((void *)(addr)));
    break;
  default:
    kprintf("Unsupported relocation %d\n", (int)(ELF32_R_TYPE(rel->r_info)));
    return 1;
  }
  return 0;
}
