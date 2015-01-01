
CSRCS = devlpr.c devtty.c devrd.c
CSRCS += devices.c main.c libc.c

ASRCS = p6502.s crt0.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.o) $(CSRCS:.c=.s) $(ASRCS:.s=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o ../fuzix.bin --mapfile ../fuzix.map -C ld65.cfg crt0.o commonmem.o \
	p6502.o ../start.o ../version.o ../lowlevel-6502.o \
	tricks.o main.o ../timer.o ../kdata.o devrd.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../bank16k.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_exec.o \
	../usermem.o devlpr.o devtty.o libc.o 
