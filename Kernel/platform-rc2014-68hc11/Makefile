
CSRCS = devtty.c
CSRCS += devices.c main.c

ASRCS = rc2014-68hc11.S crt0.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../dev/devide.c ../dev/blkdev.c
DISCARD_DSRCS = ../dev/mbr.c ../dev/devide_discard.c

COBJS = $(CSRCS:.c=$(BINEXT))
CDOBJS = $(CDSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=$(BINEXT))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.o, $(DISCARD_DSRCS))

OBJS  = $(COBJS) $(CDOBJS) $(AOBJS) $(DOBJS) $(DISCARD_DOBJS)

CROSS_CCOPTS += -I../dev/

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) $<

$(AOBJS): %$(BINEXT): %.S
	cpp -E $<  >$*.s
	$(CROSS_AS) $(ASOPTS) $*.s -o $*.o

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<

$(DISCARD_DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

clean:
	rm -f $(OBJS) *.o *.s core *~ fuzix.bin fuzix.tmpmap

image:
	$(CROSS_LD) -M -T memory.x -relax -o ../fuzix.elf crt0.o commonmem.o \
	rc2014-68hc11.o ../start.o ../version.o ../lowlevel-68hc11.o \
	tricks.o main.o discard.o ../timer.o ../kdata.o devices.o \
	blkdev.o mbr.o devide.o devide_discard.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../bank16kfc.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_fs3.o ../syscall_exec16.o \
	../usermem.o ../usermem_std-68hc11.o devtty.o >../fuzix.map
	$(CROSS_OBJCOPY) ../fuzix.elf -O binary ../fuzix.bin
