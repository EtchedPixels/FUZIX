
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rc2014-6502.s crt0.s
ASRCS += tricks.s commonmem.s

DSRCS = ../dev/devide.c ../dev/blkdev.c
DISCARD_DSRCS = ../dev/mbr.c ../dev/devide_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<
	mv ../dev/$*.o .

$(DISCARD_DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
	mv ../dev/$*.o .

clean:
	rm -f $(OBJS) *.o core *~ fuzix.img fuzix.bin

image:
	$(CROSS_LD) -o ../fuzix.bin --mapfile ../fuzix.map -C ld65.cfg crt0.o commonmem.o \
	rc2014-6502.o ../start.o ../version.o ../lowlevel-6502.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	blkdev.o mbr.o devide.o devide_discard.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bankfixed.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_fs3.o ../syscall_exec16.o \
	../usermem.o ../usermem_std-6502.o devtty.o
	# FIXME: need to leave C000-C0FF hole
	dd if=../fuzix.bin of=fuzix.imgl bs=256 skip=4 count=188
	dd if=../fuzix.bin of=fuzix.imgh bs=256 skip=193 count=62
	cat fuzix.imgl fuzix.imgh >fuzix.img
	
