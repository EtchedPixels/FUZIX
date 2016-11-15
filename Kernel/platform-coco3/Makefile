
CSRCS = ttydw.c mbr.c dwtime.c
CSRCS += devices.c main.c libc.c devsdc.c

CDSRCS = ../dev/devide_discard.c

DSRCS = ../dev/devdw.c ../dev/blkdev.c ../dev/devide.c
NSRCS = ../dev/net/net_native.c

ASRCS = coco3.s crt0.s ../platform-dragon-nx32/ide.s
ASRCS += tricks.s commonmem.s usermem_gime.s drivewire.s sdc.s videoll.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = devdw.o blkdev.o devide.o
NOBJS = net_native.o
CDOBJS = $(CDSRCS:.c=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(CDOBJS) $(NOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o) $(DSRCS:.c=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(CDOBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(NOBJS): %$(BINEXT): ../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o


clean:
	rm -f $(OBJS) $(JUNK)  core *~

image:
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../bank16k.o ../bank16k.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o devtty.o devtty.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../timer.o ../timer.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o net_native.o ../dev/net/net_native.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o video.o video.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o ../usermem.o ../usermem.c
	$(CROSS_LD) -o ../fuzix.bin -Map=../fuzix.map --script=fuzix.link --oformat=decb \
	crt0.o commonmem.o \
	coco3.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	drivewire.o devdw.o ttydw.o blkdev.o mbr.o devide.o devide_discard.o \
	../platform-dragon-nx32/ide.o devsdc.o sdc.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bank16k.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	../syscall_fs3.o \
	../usermem_std-6809.o devtty.o libc.o ../vt.o usermem_gime.o video.o \
	videoll.o dwtime.o \
	../level2.o ../syscall_level2.o ../select.o ../syscall_net.o \
	net_native.o

boot.bin: boot/boot.s
	lwasm -lboot.list -oboot.bin boot/boot.s

fuzix.dsk: boot.bin
	rm -f fuzix.dsk
	decb dskini fuzix.dsk
	decb copy -2 -b ../fuzix.bin ./fuzix.dsk,FUZIX.BIN
	decb copy -2 -b boot.bin ./fuzix.dsk,BOOT.BIN
	decb copy -l -0 -a AUTOEXEC.BAS ./fuzix.dsk,AUTOEXEC.BAS

