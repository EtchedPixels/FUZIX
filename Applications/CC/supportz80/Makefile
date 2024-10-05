all: libz80.a crt0.o

OBJ = workspace.o __true.o __switchc.o __switch.o __switchl.o __pushl.o __sex.o \
      __ldwordw.o \
      __and.o __andeq.o __or.o __oreq.o __xor.o __xoreq.o \
      __andeqde.o __oreqde.o __xoreqde.o \
      __minuseq.o __pluseq.o __pluseqde.o __minuseqde.o \
      __diveq.o __divequ.o __modeq.o __modequ.o __muleq.o \
      __bool.o __cpl.o __not.o \
      __boolc.o __cplc.o __notc.o \
      __minus.o __plus.o \
      __postinc.o __postdec.o __postincn.o __postdecn.o \
      __shr.o \
      __shleq.o __shrequ.o __shreq.o \
      __shlde.o \
      __divdeu.o __divde.o \
      __cclt.o __ccltu.o __ccgteq.o __ccgtequ.o \
      __cceq.o __ccne.o __cmpeq.o __cmpne.o __cmpeqb.o __cmpneb.o \
      __ccgt.o __ccgtu.o __cclteq.o __ccltequ.o __cmpeq0.o \
      __cmpgtu.o __cmpltu.o __cmpltequ.o __cmpgtequ.o \
      __cmpgt.o __cmplt.o __cmplteq.o __cmpgteq.o \
      __cmpgt0.o __cmplteq0.o __cmpgteq0.o __cmplt0.o \
      __andc.o __orc.o __xorc.o __andeqc.o __oreqc.o __xoreqc.o \
      __minuseqc.o __pluseqc.o __modeqc.o __modequc.o __diveqc.o __divequc.o \
      __castc.o __castcl.o __castu_l.o __cast_l.o __cast_ul.o \
      __postincc.o __postdecc.o \
      __shrdeuc.o __shrequc.o __shleqc.o __shreqc.o __shrequc.o \
      __bandl.o __borl.o __bxorl.o \
      __oreql.o __andeql.o __xoreql.o \
      __pluseql.o __minuseql.o \
      __booll.o __cpll.o __notl.o __negatel.o \
      __cmpl.o \
      __ccltl.o __ccgteql.o __ccgtl.o __cclteql.o \
      __ccltul.o __ccgtequl.o __ccgtul.o __ccltequl.o __ccequl.o __ccneul.o \
      __derefl.o \
      __assignl.o \
      __minusl.o __plusl.o __mull.o __divl.o \
      __postincl.o __postdecl.o \
      __shll.o __shrl.o __shrul.o \
      __shleql.o __shreql.o \
      __bandde.o __borde.o __bxorde.o __mulde.o __shrdeu.o \
      __muldec.o \
      __ldword.o __ldwordw.o __stword.o __stwordw.o \
      __ldbyte.o __ldbytew.o __stbyte.o __stbytew.o \
      __bcand.o __bcdiv.o __bcdivu.o __bcmul.o __bcor.o __bcshl.o __bcshr.o __bcsub.o __bcxor.o \
      __ixsub.o __iysub.o \
      _memcpy.o _memset.o _strlen.o

include ldst.mk

ldword/_10.o: makeldst
	./makeldst
	ccz80 -c ldword/_10.s

makeldst: makeldst.c
	$(CC) makeldst.c -o ./makeldst

.s.o:
	ccz80 -c $<

libz80.a: makeldst $(OBJ)
	rm -f libz80.a
	ar qc libz80.a `../lorderz80 $(OBJ) | tsort`

clean:
	rm -f *.o *.a
	rm -f ldword/* stword/*
