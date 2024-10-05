
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = 80c188.S crt0.S
ASRCS += tricks.S

DSRCS = ../../dev/devide.c ../../dev/devide_discard.c ../../dev/blkdev.c ../../dev/mbr.c
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../../dev/

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o *~ fuzix.aout

image:
	$(CROSS_LD) -M -o fuzix.aout -T fuzix.ld \
	../../start.o ../../version.o ../../lowlevel-8086.o \
	main.o ../../swap.o ../../timer.o ../../simple.o ../../kdata.o devices.o \
	../../tty.o ../../devio.o ../../filesys.o ../../process.o ../../inode.o \
	../../syscall_fs.o ../../syscall_proc.o ../../syscall_other.o ../../mm.o \
	../../devsys.o ../../usermem.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs2.o tricks.o ../../syscall_fs3.o ../../blk512.o \
	../../usermem_std-8086.o \
	devide.o devide_discard.o blkdev.o mbr.o \
	80c188.o devtty.o libc.o > ../../fuzix.map
