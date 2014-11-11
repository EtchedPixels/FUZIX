
CSRCS = devlpr.c devtty.c devrd.c
CSRCS += devices.c main.c libc.c

ASRCS = p6809.s crt0.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o ../fuzix.bin --map=../fuzix.map --script=fuzix.link \
	crt0.o commonmem.o \
	p6809.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devrd.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../single.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec.o \
	../usermem_std-6809.o devlpr.o devtty.o libc.o ../vt.o
