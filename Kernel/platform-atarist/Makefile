
CSRCS = devlpr.c devtty.c devfd.c
CSRCS += devices.c main.c libc.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
LSRCS = ../lib/68000exception.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

OBJS  = $(COBJS) $(AOBJS) $(LOBJS)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

tricks.S: ../lib/68000flat.S

image:
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	crt0.o \
	p68000.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o devices.o \
	68000exception.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../flat.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o \
	../usermem_std-68000.o devlpr.o devtty.o libc.o ../vt.o ../malloc.o \
	../font8x8.o >../fuzix.map
	m68k-uclinux-objcopy fuzix.elf -O binary ../fuzix.bin
