/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 *	UNIX shell
 *
 *	S. R. Bourne
 *	Bell Telephone Laboratories
 *
 */

#include	"defs.h"

extern int printf(const char *, ...);

/*
 *	storage allocator
 *	(circular first fit strategy)
 */

#define BUSY 01
#define busy(x)	(((intptr_t)(x)->word) & BUSY)

POS brkincr = BRKINCR;
BLKPTR blokp;			/* current search pointer */
BLKPTR bloktop;			/* top of arena (last blok) */
static uint8_t *end;		/* end of memory */

void blokinit(void)
{
        end = setbrk(0);	/* Find where space starts */
        if (((uint8_t)end) & 1)
                end = 1 + (uint8_t *)setbrk(1);	/* Align */
        bloktop = BLK(end);
/*        printf("Begin: blokop %p end %p\n", (void *)bloktop, end); */
}

ADDRESS alloc(POS nbytes)
{
	register POS rbytes = round(nbytes + BYTESPERWORD, BYTESPERWORD);

/*	printf("allocating %d [", rbytes); */
	for (;;) {
		int c = 0;
		register BLKPTR p = blokp;
		register BLKPTR q;
		do {
		        /* Only interested in holes */
			if (!busy(p)) {
			        /* Merge adjacent holes */
				while (!busy(q = p->word))
					p->word = q->word;

/*                                printf("H%d ", ADR(q) - ADR(p)); */
                                /* Big enough hole */
				if (ADR(q) - ADR(p) >= rbytes) {
				        /* blokp is the first byte after
				           the hole */
					blokp = BLK(ADR(p) + rbytes);
					/* Splitting ? */
					if (q > blokp) {
/*					        printf("S");*/
						blokp->word = p->word;
                                        }
                                        /* Update our block */
					p->word = BLK(
					        ((intptr_t)(blokp)) | BUSY);
                                        /* Usable space beyond header */
/*                                        printf("] %p\n", (void *)p); */
					return ADR(p + 1);
				};
			}
			/* Remember the last pointer */
			q = p;
			/* Move on one block */
			p = BLK(((intptr_t)(p->word)) & ~BUSY);

			if (p == q) {
			        write(2, "membad\n", 7);
			        exit(1);
                        }
/*			printf("."); */
		} while (p > q || (c++) == 0);
		/* No room at the inn - add more space */
/*		printf("+%d+", rbytes); */
		addblok(rbytes);
	}
}

static void blkfit(BLKPTR p)
{
        uint8_t *pt = (uint8_t *)&p->word;
        if (pt < (uint8_t *)brkend)
                return;
        setbrk(pt - (uint8_t *)brkend + sizeof(*p));
}

void addblok(POS reqd)
{
        /* Expanding with a local stack in the way ? */
	if (stakbas != staktop) {
		register STKPTR rndstak;
		register BLKPTR blokstak;

		/* Add the stack to the memory pool */
		pushstak(0);
		rndstak = (STKPTR) round(staktop, BYTESPERWORD);
		blokstak = BLK(stakbas) - 1;
		blokstak->word = stakbsy;
		stakbsy = blokstak;
		bloktop->word = BLK(((intptr_t)rndstak) | BUSY);
		bloktop = BLK(rndstak);
	}
	/* Round up to a chunk boundary */
	reqd += brkincr;
	reqd &= ~(brkincr - 1);
	/* Adjust */
	blokp = bloktop;
	blkfit(bloktop);
	bloktop = bloktop->word = BLK(((intptr_t)(bloktop)) + reqd);
	blkfit(bloktop);
	bloktop->word = BLK(ADR(end) + 1);
	{
		register STKPTR stakadr = STK(bloktop + 2);
		staktop = movstr(stakbot, stakadr);
		stakbas = stakbot = stakadr;
	}
}

void sh_free(void *ap)
{
	BLKPTR p = ap;

	if (p && p >= (BLKPTR)end && p < bloktop) {
	        /* Step back from data to header */
	        p--;
	        /* Clear the busy bit */
	        p->word = (BLKPTR)(((intptr_t)p->word) &  ~BUSY);
	        if (p->word == p)
	                write(2, "freebad\n", 8);
        }
}

#ifdef DEBUG
void chkbptr(BLKPTR ptr)
{
	int exf = 0;
	register BLKPTR p = end;
	register BLKPTR q;
	int us = 0, un = 0;

	for (;;) {
		q = ((intptr_t)(p->word)) & ~BUSY;
		if (p == ptr) {
			exf++;
		}
		if (q < end || q > bloktop) {
			abort(3);
		}
		if (p == bloktop) {
			break;
		}
		if (busy(p)
		    ) {
			us += q - p;
		} else {
			un += q - p;
			;
		}
		if (p >= q) {
			abort(4);
		}
		p = q;
	}
	if (exf == 0) {
		abort(1);
	}
	prn(un);
	prc(SP);
	prn(us);
	prc(NL);
}
#endif
