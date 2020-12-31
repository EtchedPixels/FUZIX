
CSRCS = devlpr.c devtty.c dma.c devfd.c devacsi.c
CSRCS += devices.c main.c libc.c

ASRCS = p68000.S crt0.S
ASRCS += tricks.S
ASRCS += cartridge.S

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
	$(QUIET_CC)$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(QUIET_AS)$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(QUIET_CC)$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(QUIET_CC)$(CROSS_CC) $(CROSS_CCOPTS) -c $<

FOBJS = crt0.o \
	p68000.o ../start.o ../version.o ../lowlevel-68000.o \
	tricks.o main.o ../timer.o ../kdata.o devfd.o devices.o \
	68000exception.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../flat.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o \
	../usermem_std-68000.o devlpr.o devtty.o libc.o ../vt.o ../malloc.o \
	blkdev.o mbr.o devide.o devide_discard.o devacsi.o dma.o \
	../font8x8.o

clean:
	$(QUIET_RM)rm -f *.o *.elf core *~ loader

tricks.S: ../lib/68000flat.S

image:	loader fuzix.ld $(FOBJS)
	$(QUIET_LD)$(CROSS_LD) -M -o fuzix.elf -T fuzix.ld $(FOBJS) >../fuzix.map
	$(CROSS_COMPILE)objcopy fuzix.elf -O binary ../fuzix.bin
	# And now make a bootable floppy
	cat loader.bin ../fuzix.bin | dd bs=737280 conv=sync >fuzixboot.st

loader:	loader.S loader.ld ../tools/atariboot
	$(QUIET_AS)$(CROSS_AS) $(ASOPTS) loader.S -o loader.o
	$(QUIET_LD)$(CROSS_LD) -M -o loader.elf -T loader.ld loader.o >loader.map
	$(CROSS_COMPILE)objcopy loader.elf -O binary loader.bin
	../tools/atariboot loader.bin

../fuzix.bin: image
cartridge.S: ../fuzix.bin

include ../quiet.mk
