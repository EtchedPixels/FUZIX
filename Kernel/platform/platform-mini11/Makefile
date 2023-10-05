
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = mini11.S crt0.S sci.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../../dev/tinydisk.c ../../dev/tinysd.c
DNSRCS = ../../dev/net/net_w5x00.c
DISCARD_DSRCS = ../../dev/tinydisk_discard.c ../../dev/tinysd_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=$(BINEXT))
A68OBJS = $(patsubst ../../dev/68hc11/%.S,%.o, $(A68SRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DNOBJS = $(patsubst ../../dev/net/%.c,%.o, $(DNSRCS))
DAOBJS = $(DASRCS:.S=$(BINEXT))
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(A68OBJS) $(DOBJS) $(DAOBJS) $(DISCARD_DOBJS) $(DNOBJS)

CROSS_CCOPTS += -I../../dev/ -I../../dev/68hc11/ -I../../dev/net/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(AOBJS): %$(BINEXT): %.S
	cpp -E $<  >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s -o $*.o

$(DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DNOBJS): %$(BINEXT): ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DAOBJS): %$(BINEXT): ../../dev/%.S
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DISCARD_DOBJS): %$(BINEXT): ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(DOBJS) $(DAOBJS) $(OBJS) *.o *.s core *~ fuzix.bin fuzix.tmpmap
	rm -f loadsd.elf loadsd

image: loadsd
	$(CROSS_LD) -M -T memory.x -relax -o ../../fuzix.elf crt0.o commonmem.o \
	mini11.o ../../start.o ../../version.o ../../cpu-68hc11/lowlevel-68hc11-flat.o \
	tricks.o main.o discard.o ../../timer.o ../../kdata.o devices.o \
	tinydisk.o tinydisk_discard.o tinysd.o tinysd_discard.o \
	sci.o ../../syscall_net.o ../../network.o net_w5x00.o \
	../../devio.o ../../filesys.o ../../process.o ../../inode.o ../../syscall_fs.o \
	../../syscall_proc.o ../../syscall_other.o ../../mm.o ../../swap.o ../../bankfixed.o \
	../../tty.o ../../devsys.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_exec.o ../../syscall_exec16.o ../../blk512.o ../../memalloc_none.o \
	../../usermem.o ../../cpu-68hc11/usermem_std-68hc11-flat.o devtty.o >../../fuzix.map
	$(CROSS_OBJCOPY) ../../fuzix.elf -O binary ../../fuzix.bin

loadsd: loadsd.S loadsd.x
	$(CROSS_CC) -c loadsd.S
	$(CROSS_LD) -M -T loadsd.x -relax -o loadsd.elf loadsd.o >loadsd.map
	$(CROSS_OBJCOPY) loadsd.elf -O binary loadsd

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: loadsd
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/sd.img bs=40017920 conv=sync
	# Add the bootstrap loader
	dd if=loadsd of=$(IMAGES)/sd.img conv=notrunc
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/sd.img bs=512 seek=2048 conv=notrunc
	# Add the kernel
	dd if=../../fuzix.bin of=$(IMAGES)/sd.img bs=512 seek=1 conv=notrunc
