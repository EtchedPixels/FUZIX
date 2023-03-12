#include <kernel.h>

/*
 *	We have two blocks. block 0 is the code, block 1 is the data.
 *	We need to relocate accordingly. It's possible that the underlying
 *	memory manager used one map but if so the addresses will be
 *	contiguous and it'll just work treating it as two banks
 */
unsigned plt_relocate(struct exec *bf)
{
	/* Relocations lie over the BSS as loaded */
	uint32_t *rp = (uint32_t *)(udata.u_database + bf->a_data);
	uint32_t n = bf->a_trsize / sizeof(uint32_t);
	uint32_t codebase = udata.u_codebase;
	uint32_t database = udata.u_database;

	uint32_t relend = bf->a_text + bf->a_data;

	/* We can use _uput/_uget as we set up the memory map so we know
	   it is valid */
	while (n--) {
		uint32_t *mp;
		uint32_t mv;
		uint32_t v = _ugetl(rp++);
		if (v > relend - 3) {	/* Bad relocation - should we fail ? */
/*			kprintf("R0 %d left, %p relendd %p", n, v, relend - 3 ); */
			return 1;
		}
		/* Which block holds the offset ? */
		if (v <= bf->a_text - 3)
			mp = (uint32_t *)(codebase + v);
		else if (v >= bf->a_text)
			mp = (uint32_t *)(database + v - bf->a_text);
		else {	/* Bad */
/*			kprintf("R1 %d left, %p a_data %p", n, v, bf->a_data); */
			return 1;
		}
		/* Now what are we relocating against */
		mv = _ugetl(mp);
/*		kprintf("Reloc %x:%p (%p) to ", v, mp, mv); */
		if (mv >= bf->a_text)
			mv += database - bf->a_text;
		else
			mv += codebase;
		/* Write the updated value */
/*		kprintf("%p\n", mv); */
		_uputl(mv, mp);
	}
	return 0;
}
