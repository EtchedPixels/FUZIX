CSRCS = beep.c blockio.c display.c doscall.c editcor.c exec.c find.c
CSRCS += flexcall.c gemcall.c globals.c insert.c main.c misc.c modify.c
CSRCS += move.c rmxcall.c ucsd.c undo.c unixcall.c wildargs.c

OBJS = $(SRCS,.c=.rel)

levee: $(OBJS)
	fcc -o levee $(OBJS)

$(OBJS): $(SRCS)

.c.rel:
	fcc -Os -c $(@:.rel=.c)

clean:
	rm -rf $(OBJS) *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *~ *.rel *.asm
