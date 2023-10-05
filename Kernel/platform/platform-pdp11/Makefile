
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = pdp11.S crt0.S
ASRCS += tricks.S

DSRCS = 
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -M -o fuzix.aout -T fuzix.ld \
	crt0.o \
	../start.o ../version.o ../lowlevel-pdp11.o \
	main.o ../swap.o ../timer.o ../simple.o ../kdata.o devices.o \
	../tty.o ../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	pdp11.o ../syscall_proc.o ../syscall_other.o ../mm.o \
	../devsys.o ../usermem.o ../syscall_exec.o ../syscall_exec16.o \
	../syscall_fs2.o tricks.o ../syscall_fs3.o ../blk512.o \
	../usermem_std-pdp11.o devtty.o libc.o > ../fuzix.map
	pdp11-aout-objcopy fuzix.aout -O binary ../fuzix.bin

	pdp11-aout-as boot-rx0.s -o boot-rx0.o
	pdp11-aout-ld boot-rx0.o -o boot-rx0.aout
	pdp11-aout-objcopy boot-rx0.aout -O binary boot-rx0
