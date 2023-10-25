
CSRCS = devnascom.c mm58174.c
CSRCS += devices.c main.c ide.c
DISCARD_CSRCS = discard.c

ASRCS = nascom.S crt0.S
ASRCS += tricks.S commonmem.S

NASRCS = ../../dev/80bus/nascom-vt.S ../../dev/80bus/nascom-pagemode.S
NASRCS += ../../dev/80bus/gm8x9.S
NSRCS = ../../dev/80bus/devgm8x9.c ../../dev/80bus/devgm833.c

DSRCS = ../../dev/tinydisk.c ../../dev/tinyide.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinyide_discard.c

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))
NAOBJS = $(patsubst ../../dev/80bus/%.S,%.o, $(NASRCS))
NOBJS = $(patsubst ../../dev/80bus/%.c,%.o, $(NSRCS))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
OBJS  = $(COBJS) $(AOBJS) $(DISCARD_COBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS) $(NAOBJS) $(NOBJS)

CROSS_CCOPTS += -I. -I../../dev/ -I../../dev/80bus/

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(NOBJS): %.o: ../../dev/80bus/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DAOBJS): %.o: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $<

$(NAOBJS): %.o: ../../dev/80bus/%.S
	$(CROSS_AS) $(ASOPTS) $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

clean:
	rm -f $(OBJS) core *~ *.o

image:
	# Note: if the code gets a tiny bit bigger then S will be over C000
	# and the -S 0xC000 needs removing. Annoyingly it's right on the
	# edge.
	$(CROSS_LD) -b -C 0x0100 -8 0xFC00 -S 0xC000 -f CLDBSbXs -o ../../fuzix.bin \
	crt0.o commonmem.o nascom.o ../../dev/80bus/nascom-pagemode.o \
	../../start.o ../../version.o \
	../../cpu-z80u/lowlevel-z80u.o ../../cpu-z80u/usermem_std-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../dev/80bus/devgm8x9.o ../../dev/80bus/gm8x9.o ide.o \
	../../dev/80bus/nascom-vt.o devnascom.o ../../dev/80bus/devgm833.o mm58174.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../vt.o ../../mm.o \
	../../mm/memalloc_none.o ../../swap.o ../../mm/bankfixed.o \
	../../devsys.o 	../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../dev/tinyide.o ../../dev/tinyide_discard.o \
	/opt/ccz80/lib/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: image
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make the kernel into a floppy  (CP/M 77 track single sided 512 bytes/sector)
	dd if=/dev/zero of=$(IMAGES)/boot.fd bs=394240 count=1
	dd if=../../fuzix.bin bs=256 skip=1 of=$(IMAGES)/boot.fd conv=notrunc
	# Put the commondata somewhere we can move up on boot
	dd if=../../fuzix.bin bs=256 skip=252 seek=223 count=4 of=$(IMAGES)/boot.fd conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
