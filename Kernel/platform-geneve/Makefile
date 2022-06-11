
CSRCS = devtty.c devinput.c
CSRCS += devices.c main.c vdp.c v99xx.c

ASRCS = geneve.S crt0.S keyboard.S
ASRCS += tricks.S commonmem.S

CDSRCS = discard.c

DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/xtkbd.c
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
	$(CROSS_AS) $(ASOPTS) $*.s

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEG1) -c $<
	mv ../dev/$*.o .

$(DISCARD_DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<
	mv ../dev/$*.o .

clean:
	rm -f $(OBJS) *.o core *~ fuzix.bin fuzix.tmpmap
	rm -f commonmem.s crt0.s rc2014-9995.s tricks.s

image:
	$(CROSS_LD) -A2 -b -C 0x0200 -S 0xF200 -Z 0xF000 -o ../fuzix.bin -m fuzix.tmpmap crt0.o commonmem.o \
	geneve.o ../start.o ../version.o ../lowlevel-tms9995.o \
	tricks.o main.o discard.o ../timer.o ../kdata.o devices.o \
	blkdev.o mbr.o devide.o devide_discard.o \
	vdp.o v99xx.o ../vt.o ../font6x8.o keyboard.o xtkbd.o \
	devinput.o ../devinput.o ../blk512.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../bank8k.o \
	../tty.o ../devsys.o ../syscall_fs2.o ../syscall_fs3.o \
	../syscall_exec.o ../syscall_exec16.o \
	../usermem.o ../usermem_std-tms9995.o devtty.o \
	/opt/cc9995/lib/libc.a /opt/cc9995/lib/lib9995.a
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../fuzix.map

LOADER0: loader.S
	$(CROSS_CC) -tti994a -s loader.s -o loader

IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage: LOADER0
	# Make a blank disk image with partition
	dd if=$(FUZIX_ROOT)/Standalone/filesystem-src/parttab.40M of=$(IMAGES)/disk.img bs=40017920 conv=sync
	# Add the file system
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/disk.img bs=512 seek=2048 conv=notrunc
	# Make an emulator image of it
	cat $(FUZIX_ROOT)/Standalone/filesystem-src/idehdr.40M $(IMAGES)/disk.img > $(IMAGES)/emu-ide.img
