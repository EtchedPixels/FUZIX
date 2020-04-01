LIBPATH=../../Library/libs
LIBC=$(LIBPATH)/libc8080.a $(ACK_ROOT)/share/ack/cpm/libem.a

CSRCS += devices.c main.c devtty.c devinput.c
DISCARD_CSRCS = discard.c

DISCARD_DSRCS = ../dev/devide_discard.c ../dev/mbr.c ../dev/ds1302_discard.c
DSRCS = ../dev/devide.c ../dev/blkdev.c ../dev/ds1302.c

CROSS_CCOPTS += -I../dev/

ASRCS = crt0.s commonmem.s ppide.s rc2014-8085.s tricks.s end.s vdp1.s
DASRCS = ../dev/ds1302_8085.s

AOBJS = $(ASRCS:.s=.o)
COBJS = $(CSRCS:.c=.o)
DISCARD_COBJS = $(DISCARD_CSRCS:.c=.o)
DISCARD_DOBJS = $(patsubst ../dev/%.c,%.o, $(DISCARD_DSRCS))
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))
DAOBJS = $(patsubst ../dev/%.s,%.o, $(DASRCS))

OBJS  = $(AOBJS) $(COBJS) $(DISCARD_COBJS) $(DISCARD_DOBJS) $(DAOBJS) $(DOBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DISCARD_COBJS): %.o: %.c
	$(CROSS_CC) -Ras=../tools/discard85 $(CROSS_CCOPTS) $<

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

$(DAOBJS): %.o: ../dev/%.s
	$(CROSS_AS) $(ASOPTS) $<

# Need to make discard work yet
$(DISCARD_DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) -Ras=../tools/discard85 $(CROSS_CCOPTS) $<

tricks.s: ../lib/8080fixedbank.s ../lib/8080fixedbank-core.s

clean:
	rm -f $(OBJS) $(JUNK)  core *~ bootblock.bin bootblock fuzix.bin

image: 
	$(CROSS_LD) -b0:0x0100 -b4:0xE800 -b5:0xD800 -o fuzix.bin crt0.o devices.o main.o \
	discard.o commonmem.o tricks.o rc2014-8085.o devtty.o \
	../start.o ../version.o ../lowlevel-8085.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec16.o  \
	ppide.o devinput.o ../devinput.o \
	blkdev.o mbr.o devide.o devide_discard.o \
	ds1302.o ds1302_discard.o ds1302_8085.o \
	../usermem_std-8085.o $(LIBC) end.o
	anm fuzix.bin | ../tools/8080map > ../fuzix.map
	../tools/ack2kernel -v fuzix.bin ../fuzix.bin
	../tools/check8080 fuzix.bin
