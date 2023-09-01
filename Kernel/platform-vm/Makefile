LIBPATH=../../Library/libs
# FIXME: paths
LIBC=$(LIBPATH)/libcvm.a /opt/cc85/lib/libvm.a

CSRCS += devices.c main.c devtty.c disk.c

ASRCS = crt0.S commonmem.S vm.S tricks.S

CROSS_CCOPTS += -I../dev/


AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

OBJS  = $(AOBJS) $(COBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

tricks.s: ../lib/8080fixedbank.s ../lib/8080fixedbank-core.s

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock

bootblock: bootblock.s
	cc85 -c bootblock.s
	ld85 -b -C 0 -o bootblock bootblock.o

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

image: bootblock
	$(CROSS_LD) -b -C 0x0000 -S 0xE800 -o fuzix.bin crt0.o devices.o main.o \
	commonmem.o tricks.o vmo devtty.o \
	../start.o ../version.o ../lowlevel-8080.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec.o ../syscall_exec16.o  \
	../usermem_std-8080.o ../blk512.o devfd.o $(LIBC) -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../fuzix.map
	../tools/pack85 <../fuzix.map fuzix.bin ../fuzix.bin

diskimage:
	# Boot floppy
	dd if=bootblock of=$(IMAGES)/boot.dsk bs=256256 conv=sync
	dd if=../fuzix.bin of=$(IMAGES)/boot.dsk bs=1 seek=193024 conv=notrunc
	# Root hard disk
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/hd-fuzix.dsk 128 8160)
