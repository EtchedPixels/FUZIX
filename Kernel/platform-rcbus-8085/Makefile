LIBPATH=../../Library/libs

CSRCS += devices.c main.c devtty.c devinput.c devscsi.c
DISCARD_CSRCS = discard.c

DISCARD_DSRCS = ../dev/tinyide_discard.c ../dev/ds1302_discard.c
DISCARD_DSRCS += ../dev/tinydisk_discard.c ../dev/tinyscsi_discard.c

DSRCS = ../dev/tinyide.c ../dev/tinydisk.c ../dev/ds1302.c ../dev/ch375.c
DSRCS += ../dev/tinyscsi.c

CROSS_CCOPTS += -I../dev/

ASRCS = crt0.S commonmem.S ppide.S rcbus-8085.S tricks.S vdp1.S ncr5380.S
DASRCS = ../dev/ds1302_8085.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.S,%.o, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(DAOBJS) $(DOBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -Tdiscard $<

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DAOBJS): %.o: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $<

# Need to make discard work yet
$(DISCARD_DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -Tdiscard $<

tricks.S: ../lib/8085fixedbank.S ../lib/8085fixedbank-core.S

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock fuzix.bin loader.tmp

image: 
	$(CROSS_LD) -b -C 0x0100 -S 0xE000 -o fuzix.bin crt0.o devices.o main.o \
	discard.o commonmem.o tricks.o rcbus-8085.o devtty.o \
	../start.o ../version.o ../lowlevel-8085.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../vt.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec.o ../syscall_exec16.o  \
	devinput.o ../devinput.o ../blk512.o \
	../dev/tinydisk_discard.o ../dev/tinydisk.o ../dev/tinyide_discard.o \
	../dev/tinyide.o ppide.o ../dev/ch375.o vdp1.o ../font6x8.o \
	../dev/tinyscsi_discard.o ../dev/tinyscsi.o devscsi.o ncr5380.o \
	../dev/ds1302.o ../dev/ds1302_discard.o ../dev/ds1302_8085.o \
	../usermem_std-8085.o -m fuzix.tmpmap /opt/cc85/lib/lib8085.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../fuzix.map
	../tools/pack85 <../fuzix.map fuzix.bin ../fuzix.bin

loader.o: loader.S
	$(CROSS_AS) $(ASOPS) loader.S

loader: loader.o
	$(CROSS_LD) -b -C 0xFE00 loader.o -o loader.tmp
	dd if=loader.tmp of=loader bs=256 skip=254

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loader
	dd if=../fuzix.bin of=fuzix.img bs=256 skip=1
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
