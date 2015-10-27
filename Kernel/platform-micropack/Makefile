
DSRCS = ../dev/z80pack/devlpr.c ../dev/z80pack/devfd.c
CSRCS = devices.c main.c devtty.c
CBSRCS = syscall_bank.c

ASRCS = crt0.s z80pack.s
ASRCS += tricks.s commonmem.s

AOBJS = $(ASRCS:.s=.rel)
COBJS = $(CSRCS:.c=.rel)
CBOBJS = $(CBSRCS:.c=.rel)
DOBJS = $(patsubst ../dev/z80pack/%.c,%.rel, $(DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(CBOBJS)

CROSS_CCOPTS += -I../dev/z80pack/

JUNK = *.rel *.lst *.asm *.sym *.rst

all:	$(OBJS)

syscall_bank.h: ../include/syscall_name.h ../syscall_fs.c ../syscall_fs2.c ../syscall_fs3.c ../syscall_other.c ../syscall_exec16.c
	../tools/map_syscall ../syscall_fs.c ../syscall_fs2.c ../syscall_fs3.c ../syscall_other.c ../syscall_exec16.c >syscall_bank.h

syscall_bank.c: syscall_bank.h

$(AOBJS): %.rel: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CBOBJS): %.rel: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) --constseg DATAAL -c $<

$(DOBJS): %.rel: ../dev/z80pack/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
