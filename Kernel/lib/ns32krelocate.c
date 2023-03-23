#include <kernel.h>
#include <printf.h>

/*
 *	We have two blocks. block 0 is the code, block 1 is the data.
 *	We need to relocate accordingly. It's possible that the underlying
 *	memory manager used one map but if so the addresses will be
 *	contiguous and it'll just work treating it as two banks
 *
 *	NS32K needs some special handling as it can have fixups that
 *	are wrong endian and fixups that are right endian.
 */
unsigned plt_relocate(struct exec *bf)
{
	unsigned arseendian;
	uint32_t sizebits;

	/* Relocations lie over the BSS as loaded */
	uint32_t *rp = (uint32_t *)(udata.u_codebase + bf->a_data + bf->a_text);
	uint32_t n = bf->a_trsize / sizeof(uint32_t);
	uint32_t codebase = udata.u_codebase;

	uint32_t relend = bf->a_text + bf->a_data;

	/* We can use _uput/_uget as we set up the memory map so we know
	   it is valid */
	while (n--) {
		uint32_t *mp;
		uint32_t mv;
		uint32_t v = _ugetl(rp++);

		/* Top bit set indicates relocation is wrong-endian */
		if (v & 0x80000000)
			arseendian = 1;
		else
			arseendian = 0;
		v &= 0x7FFFFFFF;

		if (v > relend - 3) {	/* Bad relocation - should we fail ? */
/*			kprintf("R0 %d left, %p relendd %p", n, v, relend - 3 ); */
			return 1;
		}
		mp = (uint32_t *)(codebase + v);

		/* Now what are we relocating against */
		mv = _ugetl(mp);

		if (arseendian)
			mv = ntohl(mv);

                /* If the target top bits are high then it's a 30bit
                   signed relocation */
		sizebits = 0;
		if ((mv & 0xC0000000) == 0xC0000000)
			sizebits = 1;
		/* We don't deal with underflows on the sized 30 bit signed relocs
		   We should never ever get one ; TODO review */
		mv &= 0x3FFFFFFF;
/*		kprintf("Reloc %x:%p (%p) to ", v, mp, mv); */

		mv += codebase;
		/* Write the updated value */
/*		kprintf("%p\n", mv); */
		/* Put back the field size info */
		if (sizebits)
			mv |= 0xC0000000;
		if (arseendian)
			mv = ntohl(mv);
		_uputl(mv, mp);
	}
	return 0;
}
