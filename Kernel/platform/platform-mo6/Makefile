CSRCS = devices.c main.c devinput.c

C3SRCS = libc.c

CDSRCS = discard.c

DSRCS =	../dev/tinydisk.c ../dev/tinysd.c
DDSRCS = ../dev/tinydisk_discard.c ../dev/tinysd_discard.c

ASRCS = crt0.s mo6.s usermem.s
ASRCS += tricks.s commonmem.s

TSRCS = ../dev/thomson/devfdc.c ../dev/thomson/devtty.c
TASRCS = ../dev/thomson/video-to8.s ../dev/thomson/sdcard.s

COBJS = $(CSRCS:.c=.o)
C3OBJS = $(C3SRCS:.c=.o)
CDOBJS = $(CDSRCS:.c=.o)
AOBJS = $(ASRCS:.s=.o)
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DDOBJS = $(patsubst ../dev/%.c,%.o, $(DDSRCS))
TOBJS = $(patsubst ../dev/thomson/%.c,%.o, $(TSRCS))
TAOBJS = $(patsubst ../dev/thomson/%.s,%.o, $(TASRCS))
OBJS  = $(COBJS) $(C3OBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DDOBJS) $(TOBJS) $(TAOBJS)

CROSS_CCOPTS += -I../dev/ -I../dev/thomson/

all: $(OBJS)

export CROSS_CC_SEGD=-mcode-section=.text3
export CROSS_CC_SEGI=-mcode-section=.text
export CROSS_CC_SEGX=-mcode-section=.text4

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGX) -c $<

$(C3OBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGX) -c $<

$(CDOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGD) -c $<

$(DDOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(TOBJS): %.o: ../dev/thomson/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG2) -c $<

$(TAOBJS): %.o: ../dev/thomson/%.s
	$(CROSS_AS) $(ASOPTS)  $< -o $*.o

$(AOBJS): %.o: %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f *.o *~ boot.bin boot.tmp boot.tmp2 fuzix.bin fuzix.img fuzix.tmp 

image:
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o \
	mo6.o discard.o ../bankfixed.o usermem.o ../memalloc_none.o \
	../start.o ../version.o ../lowlevel-6809.o ../usermem_std-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o tinydisk.o tinysd.o \
	sdcard.o tinydisk_discard.o tinysd_discard.o devfdc.o devinput.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../blk512.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	devtty.o libc.o ../vt.o video-to8.o ../syscall_fs3.o ../syscall_exec.o \
	../devinput.o
	../tools/decb-image <../fuzix.bin fuzix.img
	../tools/lw-checkmap ../fuzix.map
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Write a boot floppy
	cp fuzix.bin $(IMAGES)/boot.fd
