LIBPATH=../../Library/libs

CSRCS += devices.c main.c devtty.c
DISCARD_CSRCS = discard.c

DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinysd_discard.c
DSRCS = ../../dev/tinydisk.c ../../dev/tinysd.c 

CROSS_CCOPTS += -I../../dev/

ASRCS = crt0.S commonmem.S rcbus-z8.S tricks.S
DASRCS =

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(DAOBJS) $(DOBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -Tdiscard $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DAOBJS): %.o: ../../dev/%.s
	$(CROSS_AS) $(ASOPTS) $<

# Need to make discard work yet
$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -Tdiscard $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock fuzix.bin loader.tmp

image: 
	$(CROSS_LD) -t -b -C 0x0100 -S 0xF000 -f CLDBbXSs -o fuzix.bin crt0.o devices.o main.o \
	discard.o commonmem.o tricks.o rcbus-z8.o devtty.o \
	../../start.o ../../version.o ../../cpu-z8/lowlevel-z8.o \
	../../mm/bankfixed.o ../../timer.o ../../kdata.o  ../../mm/memalloc_none.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o \
	../../tty.o ../../devsys.o ../../usermem.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_exec.o ../../syscall_exec16.o  \
	../../dev/tinydisk.o ../../dev/tinysd.o \
	../../dev/tinydisk_discard.o ../../dev/tinysd_discard.o \
	../../blk512.o ../../cpu-z8/usermem_std-z8.o -m fuzix.tmpmap /opt/fcc/lib/z8/libz8.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin

loader.o: loader.S
	$(CROSS_AS) $(ASOPS) loader.S

loader: loader.o
	$(CROSS_LD) -b -C 0xFE00 loader.o -o loader.tmp
	dd if=loader.tmp of=loader bs=256 skip=254

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loader
	dd if=../../fuzix.bin of=fuzix.img bs=256 skip=1
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loader of=$(IMAGES)/disk.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=fuzix.img of=$(IMAGES)/disk.img bs=512 seek=1 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
