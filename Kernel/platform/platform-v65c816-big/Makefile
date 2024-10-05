
CSRCS = devtty.c devhd.c
CSRCS += devices.c main.c

LSRCS = ../lib/65c816.s
LOBJS = $(patsubst ../lib/%.s,%.o, $(LSRCS))

NSRCS = ../dev/net/net_native.c
NOBJS = $(patsubst ../dev/net/%.c,%.o, $(NSRCS))

ASRCS = v65.s crt0.s
ASRCS += commonmem.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS) $(LOBJS) $(NOBJS)

JUNK = $(CSRCS:.c=.o) $(CSRCS:.c=.s) $(ASRCS:.s=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(NOBJS): %$(BINEXT): ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -o $@ $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

$(LOBJS): %$(BINEXT): ../lib/%.s
	$(CROSS_AS) $(ASOPTS) $< -o $*$(BINEXT)

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o ../fuzix.bin --mapfile ../fuzix.map -C ld65.cfg \
	crt0.o commonmem.o \
	v65.o ../start.o ../version.o ../lowlevel-65c816.o \
	65c816.o main.o ../timer.o ../kdata.o devhd.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bank65c816.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_fs3.o \
	../syscall_exec16.o ../usermem.o ../usermem_std-65c816.o devtty.o \
	../syscall_net.o net_native.o ../level2.o ../syscall_level2.o \
	../select.o
	dd if=../fuzix.bin of=fuzix.img bs=256 skip=255 count=1
	dd if=/dev/zero of=fuzix.img bs=256 seek=1  count=2
	dd if=fuzix.i of=fuzix.img bs=256 seek=3 count=239
	dd if=fuzix.d of=fuzix.img bs=256 skip=3 seek=242 count=10
