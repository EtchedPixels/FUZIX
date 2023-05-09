LIBPATH=../../Library/libs
LIBC=$(LIBPATH)/libc8080.a $(ACK_ROOT)/share/ack/cpm/libem.a

CSRCS += devices.c main.c devtty.c devfd.c

ASRCS = crt0.s commonmem.s v8080.s tricks.s end.s

AOBJS = $(ASRCS:.s=.o)
COBJS = $(CSRCS:.c=.o)

OBJS  = $(AOBJS) $(COBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

tricks.s: ../lib/8080fixedbank.s ../lib/8080fixedbank-core.s

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock

bootblock: bootblock.s
	ack -mfuzix -c bootblock.s
	$(ACK_ROOT)/lib/ack/em_led -btext:0 -o bootblock.bin -s bootblock.o
	aslod <bootblock.bin >bootblock

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

image: bootblock
	$(CROSS_LD) -b0:0x0100 -b4:0xE800 -o fuzix.bin crt0.o devices.o main.o \
	commonmem.o tricks.o v8080.o devtty.o \
	../start.o ../version.o ../lowlevel-8080.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec.o ../syscall_exec16.o  \
	../usermem_std-8080.o ../blk512.o devfd.o $(LIBC) end.o
	anm fuzix.bin | ../tools/8080map > ../fuzix.map
	../tools/ack2kernel -v fuzix.bin ../fuzix.bin
	../tools/check8080 fuzix.bin

diskimage:
	# Boot floppy
	dd if=bootblock of=$(IMAGES)/boot.dsk bs=256256 conv=sync
	dd if=../fuzix.bin of=$(IMAGES)/boot.dsk bs=1 seek=193024 conv=notrunc
	# Root hard disk
	(cd $(FUZIX_ROOT)/Standalone/filesystem-src; ./build-filesystem $(ENDIANFLAGH) $(IMAGES)/hd-fuzix.dsk 128 8160)
