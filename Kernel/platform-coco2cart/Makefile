
CSRCS = devices.c main.c

# Code that must be present for the bootstrap
CISRCS = mini_ide.c devsdc.c

C3SRCS = libc.c devtty.c

CDSRCS = discard.c mini_ide_discard.c

DSRCS = ../dev/devdw.c

ASRCS = crt0.s coco2.s ide.s usermem.s sdc.s
ASRCS += bootstrap.s bootstrap_sdc.s #bootstrap_dw.s
ASRCS += tricks.s commonmem.s ../dev/drivewire/drivewire.s video.s

COBJS = $(CSRCS:.c=$(BINEXT))
CIOBJS = $(CISRCS:.c=$(BINEXT))
C3OBJS = $(C3SRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
OBJS  = $(COBJS) $(C3OBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(CIOBJS)

CROSS_CCOPTS += -I../dev/

all: $(OBJS)

export CROSS_CC_SEGD=-mcode-section=.text3

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(CIOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(C3OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGD) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGD) -c $<

$(DDOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f *.o *~

image:
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link \
	--oformat=decb \
	crt0.o bootstrap.o commonmem.o coco2.o discard.o ../simple.o \
	../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	../dev/drivewire/drivewire.o devdw.o mini_ide.o mini_ide_discard.o ide.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o usermem.o ../syscall_fs2.o \
	../syscall_exec16.o devtty.o libc.o ../vt.o video.o  ../syscall_fs3.o \
	../font8x8.o devsdc.o sdc.o
	../tools/lw-checkmap ../fuzix.map
	../tools/decb-image <../fuzix.bin fuzix.img
	dd if=fuzix.img of=fuzix.rom bs=1024 skip=48 count=16
	dd if=fuzix.img of=fuzix.ide bs=256 skip=26 count=102
	dd if=fuzix.img of=fuzix.ide bs=256 skip=1 count=1 conv=notrunc

	$(CROSS_LD) -o ../fuzix-sdc.bin -Map=../fuzix-sdc.map --script=fuzix.link \
	--oformat=decb \
	crt0.o bootstrap_sdc.o commonmem.o coco2.o discard.o ../simple.o \
	../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	../dev/drivewire/drivewire.o devdw.o mini_ide.o mini_ide_discard.o ide.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o usermem.o ../syscall_fs2.o \
	../syscall_exec16.o devtty.o libc.o ../vt.o video.o  ../syscall_fs3.o \
	../font8x8.o devsdc.o sdc.o
	../tools/decb-image <../fuzix-sdc.bin fuzix-sdc.img
	dd if=fuzix-sdc.img of=fuzixsdc.rom bs=1024 skip=48 count=16
	dd if=fuzix-sdc.img of=fuzix.sdc bs=256 skip=26 count=102
	dd if=fuzix-sdc.img of=fuzix.sdc bs=256 skip=1 count=1 conv=notrunc
