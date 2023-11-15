CROSS_CCOPTS += -I../../dev/

CSRCS = devtty.c devices.c main.c devdisk.c devrtc.c

DISCSRCS = discard.c

ASRCS = z80-mbc2.S crt0.S
ASRCS += tricks.S commonmem.S

DISCARD_DSRCS = 
DSRCS = ../../dev/tinydisk.c ../../dev/tinydisk_discard.c
NSRCS =

DASRCS =

COBJS = $(CSRCS:.c=.o)
AOBJS = $(ASRCS:.S=.o)
NOBJS = $(patsubst ../../dev/net/%.c,%.o, $(NSRCS))
DISCOBJS = $(DISCSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../../dev/%.S,%.o, $(DASRCS))

OBJS  = $(COBJS) $(AOBJS) $(NOBJS) $(DISCOBJS) $(DOBJS) $(DISCARD_DOBJS) $(DAOBJS)

JUNK =  *.o

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCOBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DISCARD_DOBJS): %.o: ../../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $(CROSS_CC_SEGDISC) -c $<

$(NOBJS): %.o: ../../dev/net/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $<

$(DAOBJS): %.o: ../../dev/%.S
	$(CROSS_AS) $(ASOPTS) $@ $<

clean:
	rm -f $(JUNK)  core *~

image:
	$(CROSS_LD) -b -C 0x0000 -S 0xF400 -f CLDBbXSs -o fuzix.bin \
	crt0.o commonmem.o z80-mbc2.o ../../start.o \
	../../version.o ../../cpu-z80u/lowlevel-z80u.o \
	../../usermem.o tricks.o main.o discard.o \
	../../timer.o ../../kdata.o devices.o ../../devio.o \
	../../filesys.o ../../blk512.o ../../process.o \
	../../inode.o ../../syscall_exec.o ../../syscall_exec16.o \
	../../syscall_fs.o ../../syscall_fs2.o ../../syscall_fs3.o \
	../../syscall_proc.o ../../syscall_other.o ../../syscall_net.o \
	../../network.o ../../tty.o ../../mm.o ../../mm/memalloc_none.o \
	../../swap.o ../../mm/bankfixed.o ../../devsys.o \
	devtty.o ../../dev/tinydisk.o ../../dev/tinydisk_discard.o \
	devdisk.o devrtc.o \
	/opt/ccz80/lib/libz80.a -m fuzix.tmpmap
	perl -lpe '$$_=hex' fuzix.tmpmap | paste -d" " - fuzix.tmpmap  |  sort -n |  cut -d" " -f 2- >../../fuzix.map
	../../tools/pack85 <../../fuzix.map fuzix.bin ../../fuzix.bin


IMAGES = $(FUZIX_ROOT)/Images/$(TARGET)

diskimage:
	(cd ../../../Standalone/filesystem-src; ./build-filesystem $(IMAGES)/filesys.img 256 12288 )
	dd if=parttab of=$(IMAGES)/DS0N01.DSK bs=512 count=16384
	dd if=$(IMAGES)/filesys.img of=$(IMAGES)/DS0N01.DSK bs=512 seek=4096 conv=notrunc
	cp ../../fuzix.bin $(IMAGES)
