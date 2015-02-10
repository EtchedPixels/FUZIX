
CSRCS = devlpr.c devtty.c devrd.c
CSRCS += devices.c main.c libc.c

#ASRCS = p8086.s crt0.s
#ASRCS += tricks.s commonmem.s

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
	$(CROSS_LD) -o ../fuzix.bin -0 -i crt0.o commonmem.o \
	p8086.o ../start.o ../version.o ../lowlevel-8086.o \
	tricks.o main.o ../timer.o ../kdata.o devrd.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../bank16k.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_exec.o \
	../usermem.o devlpr.o devtty.o libc.o 
