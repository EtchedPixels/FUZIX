
CSRCS = devlpr.c devtty.c devfd.c
CSRCS += devices.c main.c libc.c

DSRCS = ../dev/devdw.c ../dev/blkdev.c ../dev/devide.c \
        ../dev/devide_discard.c ../dev/mbr.c \
        ../dev/devsd.c ../dev/devsd_discard.c

ASRCS = crt0.s dragon.s mem-nx32.s video.s ide.s spi.s
ASRCS += tricks.s commonmem.s usermem_sam.s floppy.s drivewire.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o)

all: $(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f $(OBJS) $(JUNK)

image:
	$(CROSS_LD) -o ../fuzix.bin --map=../fuzix.map --script=fuzix.link \
	crt0.o commonmem.o usermem_sam.o \
	dragon.o mem-nx32.o ../bankfixed.o \
	../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o floppy.o devices.o \
	drivewire.o devdw.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	devlpr.o devtty.o libc.o ../vt.o video.o ../font8x8.o \
	devide.o blkdev.o ide.o devide_discard.o mbr.o \
	devsd.o devsd_discard.o spi.o
	../tools/pad256 ../fuzix.bin
