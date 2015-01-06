.SUFFIXES: .c .rel

CSRCS = beep.c blockio.c display.c doscall.c editcor.c exec.c find.c
CSRCS += flexcall.c gemcall.c globals.c insert.c main.c misc.c modify.c
CSRCS += move.c rmxcall.c ucsd.c undo.c unixcall.c wildargs.c

OBJS = $(CSRCS:.c=.rel)

levee: $(OBJS)
	fcc -o levee $(OBJS)

$(OBJS): $(CSRCS)

.c.rel:
	fcc -O3 -c $(@:.rel=.c)

clean:
	rm -rf $(OBJS) *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *~ *.rel *.asm levee levee.bin