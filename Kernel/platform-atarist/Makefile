
CSRCS = devlpr.c devtty.c dma.c devfd.c devacsi.c
CSRCS += devices.c main.c libc.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S

LSRCS = ../lib/68000exception.c
LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

DSRCS = ../dev/mbr.c ../dev/blkdev.c ../dev/devide.c ../dev/devide_discard.c
DSRCS += ../dev/ahdi.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o *.elf core *~
	rm -f loader

tricks.S: ../lib/68000flat.S

image:	loader fuzix.ld
	$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld \
	crt0.o \
	p68000.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o devices.o \
	68000exception.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../flat.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o \
	../usermem_std-68000.o devlpr.o devtty.o libc.o ../vt.o ../malloc.o \
	blkdev.o mbr.o devide.o devide_discard.o devacsi.o dma.o \
	../font8x8.o >../fuzix.map
	m68k-uclinux-objcopy fuzix.elf -O binary ../fuzix.bin
	# And now make a bootable floppy
	cat loader.bin ../fuzix.bin | dd bs=737280 conv=sync >fuzixboot.st

loader:	loader.S loader.ld
	$(CROSS_AS) $(ASOPTS) loader.S -o loader.o
	$(CROSS_LD) -M -o loader.elf -T loader.ld loader.o >loader.map
	m68k-uclinux-objcopy loader.elf -O binary loader.bin
	../tools/atariboot loader.bin
