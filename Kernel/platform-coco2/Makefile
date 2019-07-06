
CSRCS = devices.c main.c mini_ide.c devtty.c

C3SRCS = libc.c

CDSRCS = discard.c mini_ide_discard.c

DSRCS =

ASRCS = crt0.s coco2.s ide.s
ASRCS += tricks.s commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
C3OBJS = $(C3SRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
OBJS  = $(COBJS) $(C3OBJS) $(CDOBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

all: $(OBJS)

export CROSS_CC_SEGD=-mcode-section=.text3 -mfar-code-page=2
export CROSS_CC_SEGI=-mcode-section=.page1 -mfar-code-page=2

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(C3OBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGI) -c $<

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

image: bootfuz.bin
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o \
	coco2.o discard.o ../simple.o \
	../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	mini_ide.o mini_ide_discard.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	devtty.o libc.o ../vt.o ../syscall_fs3.o
	../tools/decb-image <../fuzix.bin fuzix.img
	../tools/lw-checkmap ../fuzix.map
	# Repack the high 5K where the loader wants it
	dd if=fuzix.img of=fuzix.tmp bs=1024 skip=59
	dd if=fuzix.tmp of=fuzix.img bs=1024 seek=32 conv=notrunc

bootfuz.bin: bootloader.s
	lwasm -r -b -obootfuz.bin bootloader.s

