
DSRCS = ../../dev/z80pack/devlpr.c ../../dev/z80pack/devtty.c ../../dev/z80pack/devfd.c ../../dev/z80pack/devrtc.c
CSRCS += devices.c main.c

ASRCS = crt0.S z80pack.S
ASRCS += tricks.S commonmem.S

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)
DOBJS = $(patsubst ../../dev/z80pack/%.c,%.o, $(DSRCS))

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(NOBJS)

CROSS_CCOPTS += -I../../dev/z80pack/

all:	$(OBJS)

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %.o: ../../dev/z80pack/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o core *~ 
	rm -f bootblock.bin

image:
	$(CROSS_LD) -b -C 0x0088 -S 0xF400 -X 0xE900 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o z80pack.o main.o \
	../../start.o ../../version.o \
	../../cpu-z80u/lowlevel-z80u.o ../../cpu-z80u/usermem_std-z80u.o \
	tricks.o ../../timer.o ../../kdata.o ../../usermem.o \
	../../dev/z80pack/devfd.o devices.o ../../devio.o ../../filesys.o \
	../../blk512.o ../../process.o ../../inode.o ../../syscall_exec16.o \
	../../syscall_exec.o ../../syscall_fs.o ../../syscall_fs2.o \
	../../syscall_fs3.o ../../syscall_proc.o ../../syscall_other.o \
	../../tty.o ../../mm.o ../../mm/memalloc_none.o ../../mm/bankfixed.o \
	../../swap.o ../../devsys.o ../../dev/z80pack/devlpr.o \
	../../dev/z80pack/devtty.o ../../dev/z80pack/devrtc.o \
	/opt/ccz80/lib/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin
	asz80 bootblock.S -o bootblock.o
	ldz80 -b bootblock.o -o bootblock.bin

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	# Boot floppy
	dd if=bootblock.bin of=$(IMAGES)/boot.dsk bs=256256 conv=sync
	dd if=../../fuzix.bin of=$(IMAGES)/boot.dsk bs=1 skip=136 seek=193024 conv=notrunc
	# Root hard disk. Mostly space so just grow to size
	dd if=/dev/zero of=$(IMAGES)/drivep.dsk bs=1 seek=536870911 count=1
	# Put the data over the start
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/drivep.dsk conv=notrunc

commonmem.S: ../../cpu-z80u/std-commonmem.s
tricks.S: kernelu.def ../../cpu-z80u/kernel-z80.def ../../lib/z80ufixedbank.S
z80pack.S: kernelu.def ../../cpu-z80u/kernel-z80.def
devices.c: ../../include/kernel.h ../../include/version.h ../../include/kdata.h ../../include/tty.h \
        ../../dev/z80pack/devfd.h ../../include/devsys.h ../../dev/z80pack/devlpr.h \
        ../../include/devsys.h devtty.h ../../dev/z80pack/devrtc.h
devtty.h: ../../dev/z80pack/devtty.h
main.c: ../../include/kernel.h ../../include/timer.h ../../include/kdata.h \
        ../../include/printf.h devtty.h
../../include/kernel.h: config.h
../../cpu-z80/kernelu-z80.def: kernelu-def
../../lib/z80ufixedbank.S: ../../lib/z80ufixedbank-core.s

../../dev/z80pack/devlpr.c: ../../include/kernel.h ../../include/kdata.h \
         ../../include/printf.h ../../dev/z80pack/devlpr.h
../../dev/z80pack/devtty.c: ../../include/kernel.h ../../include/kdata.h \
         ../../include/printf.h ../../dev/z80pack/devtty.h ../../include/tty.h
../../dev/z80pack/devfd.c: ../../include/kernel.h ../../include/kdata.h \
         ../../include/printf.h ../../dev/z80pack/devfd.h ../../include/fdc.h
../../dev/z80pack/devrtc.c: ../../include/kernel.h ../../include/kdata.h \
         ../../include/printf.h ../../dev/z80pack/devrtc.h ../../include/rtc.h
