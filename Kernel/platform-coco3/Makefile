
CSRCS = devtty.c ttydw.c
CSRCS += devices.c main.c libc.c

DSRCS = ../dev/devdw.c

ASRCS = coco3.s crt0.s
ASRCS += tricks.s commonmem.s usermem_gime.s drivewire.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
DOBJS = devdw.o
OBJS  = $(COBJS) $(AOBJS) $(DOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o)

all:	$(OBJS)

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(DOBJS): %$(BINEXT): ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o


clean:
	rm -f $(OBJS) $(JUNK)  core *~

image:
	$(CROSS_CC) $(CROSS_CCOPTS) -O0 -c -o ../bank16k.o ../bank16k.c
	$(CROSS_LD) -o ../fuzix.bin --map=../fuzix.map --script=fuzix.link \
	crt0.o commonmem.o \
	coco3.o ../start.o ../version.o ../lowlevel-6809.o \
	tricks.o main.o ../timer.o ../kdata.o devices.o \
	drivewire.o devdw.o ttydw.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../bank16k.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	../usermem_std-6809.o devtty.o libc.o ../vt.o usermem_gime.o

boot.bin: boot/boot.s
	lwasm -lboot.list -oboot.bin boot/boot.s

fuzix.dsk: boot.bin
	rm -f fuzix.dsk
	decb dskini fuzix.dsk
	decb copy -2 -b ../fuzix.bin ./fuzix.dsk,FUZIX.BIN
	decb copy -2 -b boot.bin ./fuzix.dsk,BOOT.BIN
	decb copy -l -0 -a AUTOEXEC.BAS ./fuzix.dsk,AUTOEXEC.BAS

