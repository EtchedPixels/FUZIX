
CSRCS = ttydw.c
CSRCS += devices.c main.c libc.c devsdc.c

CDSRCS = 

DSRCS = ../dev/devdw.c ../dev/blkdev.c ../dev/mbr.c

ASRCS = multicomp09.s crt0.s
ASRCS += tricks.s commonmem.s usermem_gime.s ../dev/drivewire/drivewire.s sdc.s videoll.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = devdw.o blkdev.o mbr.o
CDOBJS = $(CDSRCS:.c=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(CDOBJS)

# to save intermediate files (eg, assembler) add -save-temps=cwd
# and do the same thing in cpu-6809/rules.mk
CROSS_CCOPTS += -I../dev/

../dev/drivewire/drivewire.o: ASOPTS += --defsym MULTICOMP=1

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o) $(DSRCS:.c=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s kernel.def platform.def ../kernel09.def
	$(CROSS_AS) $(ASOPTS) $< -o $*.o


clean:
	rm -f $(OBJS) $(JUNK)  core *~

image:
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../bank16k.o ../bank16k.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o devtty.o devtty.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o ../usermem.o ../usermem.c
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o \
	multicomp09.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	../dev/drivewire/drivewire.o devdw.o ttydw.o blkdev.o mbr.o \
	devsdc.o sdc.o ../memalloc_none.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bank16k.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_exec.o ../syscall_exec16.o ../syscall_fs3.o \
	../usermem_std-6809.o devtty.o libc.o ../vt.o usermem_gime.o \
	videoll.o ../blk512.o

boot.bin: boot/boot.s
	lwasm -lboot.list --format=raw -oboot.bin boot/boot.s
