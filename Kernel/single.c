#include <kernel.h>
#include <timer.h>
#include <kdata.h>
#include <printf.h>

/*
 *	Support for single tasking systems where there is not enough memory for
 *	multiple tasks and where there is no suitable swap device. This configuration
 *	is suitable for systems with over about 96K of RAM and nothing but a floppy
 *	drive.
 *
 *	To use this method of operation you should do the following
 *
 *	1. place ramtop in COMMONMEM
 *	2. base your dofork on the example in platform-socz80-lite/tricks.s
 *
 *	If you don't have a "true" common area then your code that calls swap
 *	helpers while banked will need to copy ramtop between the banks
 */



#ifndef CONFIG_MULTI

/*
 *	Platforms that only have swap and room for one process use
 *	this common logic. They must also implement forking to disk.
 *	page is used as a token, 0 means swapped, non zero means present
 */

void pagemap_free(ptptr p)
{
	p->p_page = 0;
}

int pagemap_alloc(ptptr p)
{
	p->p_page = 1;
	return 0;
}

int pagemap_realloc(uint16_t size)
{
	if (size >= (uint16_t) ramtop)
		return ENOMEM;
	udata.u_ptab->p_page = 1;
	udata.u_page = 1;
	return 0;
}

uint16_t pagemap_mem_used(void)
{
	uint16_t mem = PROGTOP - ramtop + (uint16_t)udata.u_top;
	return mem >> 10;
}

void pagemap_init(void)
{
}

#ifndef SWAPDEV

/* Force the rest of this file into common */

void dummy(void) __naked
{
	__asm.area _COMMONMEM __endasm;
}

/*
 *	Swap helpers for simple non swapping systems.
 *
 *	We always "swap" ourselves out by swapping a copy of ourself upwards in
 *	RAM and making the original the child. We are single tasking so swap is
 *	effectively a stack
 *
 *	Compressed swap to debug yet
 */

__sfr __at 0xc0 ttydata;

static void Xpanic(char *p)
{
	p;
	while (*p)
		ttydata = *p++;
}

static void Xoutch(uint16_t v)
{
	v;
	ttydata = "0123456789ABCDEF"[v & 0x0F];
}

static void Xout(uint16_t v)
{
	Xoutch(v >> 12);
	Xoutch(v >> 8);
	Xoutch(v >> 4);
	Xoutch(v);
}

#if 1
#define LCODE		0xC7
#define LCODE_QUOTED	0
#define LCODE_END	1
#define LCODE_SPARE	2

static int count(uint8_t ** sp, uint16_t * lp)
{
	uint8_t *s = *sp;
	uint8_t c = *s++;
	uint8_t ct = 3;
	uint16_t l = *lp;

	while (*s == c && l > 1 && ct < 255) {
		s++;
		ct++;
		l--;
	}
	*lp = l;
	*sp = s;
//  Xpanic("L");
//  Xout(ct);
	return ct;
}

static int do_swap_out(uint8_t * s, uint16_t l)
{
	uint8_t *p = (uint8_t *)(ramtop - 1);

//  Xpanic("do_swap_out ");
//  Xout((uint16_t)s);
//  Xpanic(",");
//  Xout((uint16_t)l);
//  Xpanic("\n");  
	while (l) {
		if ((uint16_t) p - udata.u_top < 6) {
			Xpanic("nofit");
			return ENOMEM;
		}
		if (l > 4 && *s == s[1] && *s == s[2]) {
			uint8_t c = *s;
			*p-- = LCODE;
			s += 2;
			l -= 2;
			*p-- = count(&s, &l);
			*p-- = c;
		} else if (*s == LCODE) {
			*p-- = LCODE;
			*p-- = LCODE_QUOTED;
			s++;
		} else
			*p-- = *s++;
		l--;
		if (l > 0xFFF0)
			Xpanic("overrun");
	}
//  Xpanic("E");
	/* Write an end marker */
	*p-- = LCODE;
	*p-- = LCODE_END;
	/* Write the pointer to the block beginning */
	*p-- = (uint16_t) ramtop >> 8;
	*p = (uint16_t) ramtop & 0xFF;
//  Xpanic("Ramtop was ");
//  Xout(ramtop);
	ramtop = (uint16_t)p;
//  Xpanic(" now ");
//  Xout(ramtop);
//  Xpanic("\r\n");
	s;
	l;
	return 0;
}

static void unpack(uint8_t * d, uint8_t * p, uint16_t l)
{
	int i;
	uint8_t c;

	while (l) {
		if (*p != LCODE) {
			*d++ = *p--;
			l--;
			continue;
		}
		p--;
		if (*p == LCODE_QUOTED) {
			*d++ = LCODE;
			p--;
			l--;
			continue;
		}
		if (*p == LCODE_END) {
			Xpanic("short swap");
		}
		c = p[-1];
		for (i = *p; i > 0; i--) {
			*d++ = c;
			l--;
			if (l == 0) {
				Xpanic("bogon by ");
				Xout(i);
			}
		}
		p -= 2;
	}
	if (*p != LCODE && p[-1] != LCODE_END)
		Xpanic("long swap\n");
}

static void do_swap_in(uint8_t * d, uint16_t l)
{
	uint8_t *p;
	p = (uint8_t *) ramtop;
	p = (uint8_t *) (p[0] | (((uint16_t) p[1]) << 8));	/* Recover old base pointer */
//  Xout(ramtop[1]);
//  Xout(ramtop[2]);
//  Xpanic("Entry Ramtop: ");
//  Xout((uint16_t)ramtop);
//  Xpanic("New Ramtop: ");
//  Xout((uint16_t)p);
	unpack(d, p, l);
	ramtop = (uint16_t)p;
//  Xpanic("\r\n");
}
#endif
#if 0
static void Xmemcpy(uint8_t * d, uint8_t * s, uint16_t l)
{
	while (l--)
		*d++ = *s++;
}

static int do_swap_out(uint8_t * p, uint16_t l)
{
	uint16_t room = ramtop - udata.u_top;
	uint8_t *d;

	Xpanic("len ");
	Xout(l);
	Xpanic(" room ");
	Xout(room);
	if (room < l + 2)
		return -1;
	ramtop -= l + 2;
	Xpanic("ramtop now ");
	Xout(ramtop);
	d = (uint8_t *) ramtop;
	d[0] = l & 0xFF;
	d[1] = l >> 8;
	Xmemcpy(d + 2, p, l);
	return 0;
}

static void do_swap_in(uint8_t * d, uint16_t l)
{
	uint8_t *p = (uint8_t *) ramtop;
	uint16_t sl;
	//Xpanic("len ");Xout(l);Xpanic(" ramtop ");Xout((uint16_t)ramtop);
	sl = p[0] | ((uint16_t) p[1] << 8);
	if (sl != l)
		Xpanic("ilen");
	Xmemcpy(d, p + 2, sl);
	ramtop += 2 + sl;
	Xpanic(" now ");
	Xout((uint16_t) ramtop);
}
#endif

int swapout(ptptr p)
{
	uint16_t oldtop = ramtop;
	p;
//  Xpanic("Swapout");
//  Xout((uint16_t)udata.u_ptab);
	if (do_swap_out(0, udata.u_top) == 0
	    && do_swap_out((uint8_t *) & udata, 768) == 0) {
//    Xpanic("OK");
		return 0;
	}
//  Xpanic("BAD");
	ramtop = oldtop;
	return ENOMEM;
}

ptptr swapneeded(ptptr p, int notself)
{
	p;
	notself;
	swapout(udata.u_ptab);
	return udata.u_ptab;
}

void swapper(ptptr p)
{
	/* Get the udata back */
//  Xpanic("Swapin");
	do_swap_in((uint8_t *) & udata, 768);
	if (udata.u_ptab != p)
		Xpanic("swapin");
	do_swap_in(0, udata.u_top);
//  Xpanic("udata proctab");Xout((uint16_t)udata.u_ptab);
}

#endif
#endif
