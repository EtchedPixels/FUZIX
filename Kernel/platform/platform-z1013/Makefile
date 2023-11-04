ASRCS = crt0.S tricks.S commonmem.S z1013.S fdc765.S video-poppe.S video-32x32.S
CSRCS = devices.c main.c devtty.c devrtc.c devrd.c sd.c
DISCARD_CSRCS = discard.c
DISCARD_DSRCS = ../../dev/tinyide_discard.c ../../dev/tinysd_discard.c
DISCARD_DSRCS += ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinyide.c ../../dev/tinysd.c ../../dev/devfdc765.c
DSRCS += ../../dev/tinydisk.c
DASRCS = ../../dev/z80usoftsd.S ../../dev/z80usoftspi.S
NSRCS = 

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DHOBJS = $(patsubst ../../dev/%.c,%.o, $(DHSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(DHOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DISCARD_COBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/net/

CROSS_CC_HIGH = -Tcommon

JUNK = *.o *.lst *.asm *.Sym *.rst *.map *.ihx *.bin

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_HIGH) -c $<

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
	$(CROSS_LD) -b -C 0x8000 -S 0x0100 -X 0x6200 -f CXSsLDBb -o ../../fuzix.bin \
	crt0.o commonmem.o z1013.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o devrd.o devrtc.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../vt.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/simple.o ../../devsys.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o \
	../../dev/tinysd.o ../../dev/tinysd_discard.o \
	sd.o ../../dev/z80usoftsd.o ../../dev/z80usoftspi.o \
	../../dev/devfdc765.o fdc765.o video-poppe.o video-32x32.o \
	/opt/ccz80/lib/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map

	dd if=../../fuzix.bin of=fuzix.rom bs=32768 skip=1 count=1
	# Build the disk bootstrap 
	dd if=../../fuzix.bin of=diskstrap bs=256 skip=1 count=128

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)
diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel extra blocks
	dd if=diskstrap of=$(IMAGES)/disk.img bs=512 seek=2 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
	# ROM
	cp fuzix.rom $(IMAGES)/fuzix.rom
