
CSRCS = devtty.c devhd.c
CSRCS += devices.c main.c

ASRCS = appleii.s crt0.s firmware.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.o) $(CSRCS:.c=.s) $(ASRCS:.s=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o ../../fuzix.bin --mapfile ../../fuzix.map -C ld65.cfg crt0.o commonmem.o \
	appleii.o ../../start.o ../../version.o ../../cpu-6502/lowlevel-6502.o ../../mm/simple.o \
	../../swap.o ../../mm/memalloc_none.o \
	tricks.o main.o ../../timer.o ../../kdata.o devhd.o devices.o ../../vt.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../mm/bankfixed.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec.o ../../usermem.o ../../cpu-6502/usermem_std-6502.o \
	../../syscall_exec16.o devtty.o firmware.o ../../blk512.o
	dd if=../../fuzix.bin of=fuzix.img bs=512 skip=1
