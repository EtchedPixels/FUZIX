
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c biosdisk.c bioscon.c 8259a.c

ASRCS = ibmpc.S crt0.S biosvid.S
ASRCS += tricks.S

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	crt0.o \
	../start.o ../version.o ../lowlevel-8086.o \
	main.o ../swap.o ../timer.o ../simple.o ../kdata.o devices.o \
	../tty.o ../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	ibmpc.o ../syscall_proc.o ../syscall_other.o ../mm.o \
	../devsys.o ../usermem.o ../syscall_exec16.o ../syscall_fs2.o \
	tricks.o ../syscall_fs3.o biosdisk.o ../vt.o bioscon.o biosvid.o \
	../bank8086.o 8259a.o \
	../usermem_std-8086.o devtty.o libc.o > ../fuzix.map
