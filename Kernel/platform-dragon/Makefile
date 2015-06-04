
CSRCS = devlpr.c devtty.c devfd.c
CSRCS += devices.c main.c libc.c

DSRCS = ../dev/devdw.c

ASRCS = p6809.s crt0.s drivewire.s
ASRCS += tricks.s commonmem.s usermem_sam.s floppy.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o) \
       d64_1.decb d64_1.rom d64_2.rom cart1.rom cart2.rom

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

d64_1.rom: setup.s
	lwasm -r -o d64_1.rom setup.s

image: d64_1.rom
	$(CROSS_LD) -o ../fuzix.bin --map=../fuzix.map --script=fuzix.link \
	crt0.o commonmem.o usermem_sam.o \
	p6809.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o floppy.o devices.o \
	drivewire.o devdw.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../simple.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	devlpr.o devtty.o libc.o ../vt.o
	../tools/decbdragon < ../fuzix.bin d64_1.decb d64_2.rom cart1.rom cart2.rom
	dd if=d64_1.decb of=d64_1.rom bs=1024 seek=1 conv=notrunc
