
CSRCS = ttydw.c mbr.c dwtime.c bank0.c
CSRCS += devices.c main.c libc.c devsdc.c

CDSRCS = ../dev/devide_discard.c

DSRCS = ../dev/devdw.c ../dev/blkdev.c ../dev/devide.c
NSRCS = ../dev/net/net_native.c

ASRCS = coco3.s crt0.s ../platform-dragon-nx32/ide.s
ASRCS += tricks.s commonmem.s usermem_gime.s drivewire.s sdc.s videoll.s
ASRCS += video_link.s

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

image:	kernel bank0.img

kernel:
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../bank16k.o ../bank16k.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o devtty.o devtty.c
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../timer.o ../timer.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o video.o video.c
	$(CROSS_CC) $(CROSS_CC_VIDEO) $(CROSS_CCOPTS) -O0 -c -o ../usermem.o ../usermem.c
	$(CROSS_LD) -o ../fuzix.bin --map=../fuzix.map --script=fuzix.link \
	crt0.o commonmem.o \
	coco3.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	drivewire.o devdw.o ttydw.o blkdev.o mbr.o devide.o devide_discard.o \
	../platform-dragon-nx32/ide.o devsdc.o sdc.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bank16k.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	../syscall_fs3.o \
	../usermem_std-6809.o devtty.o libc.o usermem_gime.o \
	video_link.o dwtime.o \
	../level2.o ../syscall_level2.o ../select.o ../syscall_net.o \
	net_native.o

bank0.img:
	m4 bank0.si > bank0.s
	lwasm -obank0.o --obj bank0.s
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o video.o video.c
	$(CROSS_LD) -r -o bank0.img --map=bank0.map --script=bank.link \
	../vt.o video.o videoll.o bank0.o

boot.bin: boot/boot.s
	lwasm -lboot.list -oboot.bin boot/boot.s

fuzix.dsk: boot.bin
	rm -f fuzix.dsk
	decb dskini fuzix.dsk
	decb copy -2 -b ../fuzix.bin ./fuzix.dsk,FUZIX.BIN
	decb copy -2 -b boot.bin ./fuzix.dsk,BOOT.BIN
	decb copy -l -0 -a AUTOEXEC.BAS ./fuzix.dsk,AUTOEXEC.BAS

