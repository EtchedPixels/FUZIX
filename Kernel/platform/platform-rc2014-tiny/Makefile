ASRCS = crt0.S tricks.S commonmem.S rc2014.S ide.S
CSRCS = devices.c main.c devtty.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/ds1302_discard.c ../../dev/tinyide_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinydisk.c
DSRCS +=  ../../dev/ds1302.c
DASRCS = ../../dev/ds1302_rcbusu.S
NSRCS = 

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

JUNK = *.o *.tmp *.bin

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

clean:
	rm -f $(OBJS) $(JUNK)  core *~ diskstrap fuzix.rom

image:
	$(CROSS_LD) -b -C 0x0080 -S 0xC000 -X0x8200 -f CXSsLDBb -o ../../fuzix.bin \
	crt0.o commonmem.o rc2014.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o ../../cpu-z80u/usermem_std-z80u.o \
	tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/simple.o ../../devsys.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o ide.o \
	../../dev/ds1302_rcbusu.o ../../dev/ds1302.o ../../dev/ds1302_discard.o \
	/opt/ccz80/lib/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	dd if=../../fuzix.bin of=fuzix.rom bs=32768 count=1
	# We have a 512 byte hole we leave for the loader
	dd if=../../fuzix.bin of=diskstrap bs=33280 count=1 skip=1

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	mv fuzix.rom $(FUZIX_ROOT)/Images/$(TARGET)
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# The RAM part of the OS load
	dd if=diskstrap of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
