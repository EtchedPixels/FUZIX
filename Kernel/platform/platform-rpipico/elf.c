#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <elf.h>

int plt_relocate_rel(Elf32_Rel* rel, uaddr_t section_base)
{
	switch (ELF32_R_TYPE(rel->r_info))
	{
		case R_ARM_RELATIVE:
		{
			uaddr_t addr = section_base + rel->r_offset;
			uaddr_t value = ugetp((void*) addr);
			value += section_base;
			uputp(value, (void*) addr);
			break;
		}

		default:
			kprintf("unsupported ARM relocation %d\n", ELF32_R_TYPE(rel->r_info));
			return 1;
	}
	return 0;
}

