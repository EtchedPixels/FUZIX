CROSS_CCOPTS += -I../../dev/

CHSRCS = devhd.c
CSRCS = devices.c main.c devtty.c

DISCSRCS = discard.c

ASRCS = challenger.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = ../../dev/tinydisk_discard.c
DSRCS = ../../dev/tinydisk.c

DASRCS =

COBJS = $(CSRCS:.c=.o)
CHOBJS = $(CHSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(DISCARD_DSRCS:.c=.o)
DOBJS = $(DSRCS:.c=.o)
DAOBJS = $(DASRCS:.S=.o)

OBJS  = $(COBJS) $(CHOBJS) $(AOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -Tcommon -c $<

$(CHOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

# Don't need to be in common but it balances the space better
$(DOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(JUNK)  core *~ *.bin

#
#	Attach the kernel to the boot block
#
image:	loader-6502.bin loader-z80.bin
	$(CROSS_LD) -b -C 0x0100 -S 0xD000 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o challenger.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o devhd.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	../../cpu-z80u/usermem_std-z80u.o \
	/opt/fcc/lib/z80/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin
	
#
#	Compile up the boot block
#

loader-6502.bin: loader-6502.S
	fcc -m6502 loader-6502.S -c -o loader-6502.o
	ld6502 -b loader-6502.o -o loader-6502.bin

loader-z80.bin: loader-z80.S
	fcc -mz80 loader-z80.S -c -o loader-z80.o
	ldz80 -b loader-z80.o -o loader-z80.bin
	
IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

../../tools/makeosihd: ../../tools/makeosihd.c
	gcc -Wall -pedantic ../../tools/makeosihd.c -o ../../tools/makeosihd
	
../../tools/osifloppy: ../../tools/osifloppy.c
	gcc -Wall -pedantic ../../tools/osifloppy.c -o ../../tools/osifloppy
	
diskimage: ../../tools/osifloppy ../../tools/makeosihd
	# Build the bootable floppy image
	../../tools/osifloppy 8 loader-6502.bin loader-z80.bin ../../fuzix.bin >$(IMAGES)/boot8.65u
	../../tools/osifloppy 5 loader-6502.bin loader-z80.bin ../../fuzix.bin >$(IMAGES)/boot5.65u
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.20M of=$(IMAGES)/disk.img bs=20152320 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys8.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# TODO: add the kernel (not that we have an HD boot yet)
	# Make a CD-36 out of it for winOSI
	../../tools/makeosihd cd-36 <$(IMAGES)/disk.img >$(IMAGES)/cd-36.hd
	# And a CD-74
	../../tools/makeosihd cd-74 <$(IMAGES)/disk.img >$(IMAGES)/cd-74.hd
	# Make an emulator image of it for PIA IDE
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.20M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
