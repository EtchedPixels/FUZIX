CSRCS = libc.c tm4c129x.c devtty.c devsdspi.c devices.c systick.c \
        syscall.c ssi.c procman.c eth.c gpio.c interrupt.c clock.c

ASRCS = tricks.S arm_exception.S

GCC_CSRCS = libgcc2.c
GCC_ASRCS = libgcc.S
GCC_COBJS = $(GCC_CSRCS:.c=$(BINEXT))
GCC_AOBJS = $(GCC_ASRCS:.S=.o)

LCSRCS = armrelocate.c
LOBJS = $(LCSRCS:.c=$(BINEXT))

DSRCS = blkdev.c mbr.c devsd.c devsd_discard.c
DOBJS = $(DSRCS:.c=$(BINEXT))

NSRCS = net_native.c
NOBJS = $(NSRCS:.c=$(BINEXT))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS) $(NOBJS) \
        $(GCC_AOBJS) $(GCC_COBJS)

CROSS_CCOPTS += -I../dev

all:	crt0.o $(OBJS)

crt0.o: crt0.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(GCC_COBJS): %.o: from_gcc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(GCC_AOBJS): %.o: from_gcc/%.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %.o: ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o *.elf core* *~

image:
	$(CROSS_LD) --entry=start \
	-M -o fuzix.elf -T fuzix.ld $(OBJS) \
	../start.o ../version.o ../lowlevel-armm4.o \
	../timer.o ../kdata.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_net.o ../syscall_other.o \
	../network.o ../mm.o ../swap.o ../flat.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec.o ../syscall_exec32.o \
	../syscall_level2.o ../level2.o ../select.o ../blk512.o \
	../usermem_std-armm4.o ../vt.o ../malloc.o \
	../font8x8.o >../fuzix.map

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	cp fuzix.elf $(IMAGES)
