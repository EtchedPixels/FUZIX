ASRCS = crt0.S tricks.S commonmem.S aqplus.S vt.S
CSRCS = devices.c main.c devtty.c iop.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinydisk.c
#NSRCS = ../../dev/net/net_native.c

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.o *~

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

image: loader
	$(CROSS_LD) -b -C 0x0100 -X 0xC800 -S 0xF000 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o aqplus.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../cpu-z80u/usermem_std-z80u.o \
	../../usermem.o tricks.o main.o discard.o iop.o vt.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../vt.o ../../devinput.o \
	../../mm.o ../../mm/memalloc_none.o ../../swap.o ../../mm/bank16k.o \
	../../devsys.o devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin

clean:
	rm -f $(OBJS) $(JUNK)  core *~
	rm -f fuzix.romwbw boot-romwbw.tmp

#
#	Compile up the boot block
#
loader:
	fcc -mz80 -c loader.S
	ldz80 -b loader.o -o loader.tmp
	# Chop off the leading 14561 bytes we don't want
	dd if=loader.tmp of=loader.bin bs=1 skip=14561

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the bootstrap
	dd if=boot-romwbw.bin of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# And kernel
	dd if=../../fuzix.bin of=$(IMAGES)/disk.img bs=512 seek=4 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
