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


/*
 *	storage allocator
 *	(circular first fit strategy)
 */

#define BUSY 01
#define busy(x)	(Rcheat((x)->word)&BUSY)

POS		brkincr=BRKINCR;
BLKPTR		blokp;			/*current search pointer*/
BLKPTR		bloktop=BLK(end);	/*top of arena (last blok)*/



ADDRESS	alloc(nbytes)
	POS		nbytes;
{
	REG POS		rbytes = round(nbytes+BYTESPERWORD,BYTESPERWORD);

	for (;;) {	INT		c=0;
		REG BLKPTR	p = blokp;
		REG BLKPTR	q;
		REP	IF !busy(p)
			) {	WHILE !busy(q = p->word) DO p->word = q->word OD
				IF ADR(q)-ADR(p) >= rbytes
				) {	blokp = BLK(ADR(p)+rbytes);
					IF q > blokp
					) {	blokp->word = p->word;
					FI
					p->word=BLK(Rcheat(blokp)|BUSY);
					return(ADR(p+1));
				FI
			FI
			q = p; p = BLK(Rcheat(p->word)&~BUSY);
		PER	p>q || (c++)==0 DONE
		addblok(rbytes);
	}
}

void	addblok(reqd)
	POS		reqd;
{
	IF stakbas!=staktop
	) {	REG STKPTR	rndstak;
		REG BLKPTR	blokstak;

		pushstak(0);
		rndstak=(STKPTR)round(staktop,BYTESPERWORD);
		blokstak=BLK(stakbas)-1;
		blokstak->word=stakbsy; stakbsy=blokstak;
		bloktop->word=BLK(Rcheat(rndstak)|BUSY);
		bloktop=BLK(rndstak);
	FI
	reqd += brkincr; reqd &= ~(brkincr-1);
	blokp=bloktop;
	bloktop=bloktop->word=BLK(Rcheat(bloktop)+reqd);
	bloktop->word=BLK(ADR(end)+1);
	{
	   REG STKPTR stakadr=STK(bloktop+2);
	   staktop=movstr(stakbot,stakadr);
	   stakbas=stakbot=stakadr;
	}
}

void free(void *ap)
{
	BLKPTR p;

	IF (p=ap) && p<bloktop
	) {	Lcheat((--p)->word) &= ~BUSY;
	FI
}

#ifdef DEBUG
chkbptr(ptr)
	BLKPTR	ptr;
{
	INT		exf=0;
	REG BLKPTR	p = end;
	REG BLKPTR	q;
	INT		us=0, un=0;

	for (;;) {
	   q = Rcheat(p->word)&~BUSY;
	   IF p==ptr ) { exf++ FI
	   IF q<end || q>bloktop ) { abort(3) FI
	   IF p==bloktop ) { break FI
	   IF busy(p)
	   ) { us += q-p;
	   } else { un += q-p;
	   FI
	   IF p>=q ) { abort(4) FI
	   p=q;
	}
	IF exf==0 ) { abort(1) FI
	prn(un); prc(SP); prn(us); prc(NL);
}
#endif
