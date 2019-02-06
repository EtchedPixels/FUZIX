LIBPATH=../../Library/libs
LIBC=$(LIBPATH)/libc8080.a $(ACK_ROOT)/share/ack/cpm/libem.a

CSRCS += devices.c main.c devtty.c

ASRCS = crt0.s commonmem.s v8080.s tricks.s end.s

AOBJS = $(ASRCS:.s=.o)
COBJS = $(CSRCS:.c=.o)

OBJS  = $(AOBJS) $(COBJS) $(DOBJS) $(NOBJS)

JUNK = *.o *.lst *.asm *.sym *.rst

all:	$(OBJS)

$(AOBJS): %.o: %.s
	$(CROSS_AS) $(ASOPTS) $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) $<

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o fuzix.bin crt0.o devices.o main.o \
	commonmem.o tricks.o v8080.o devtty.o \
	../start.o ../version.o ../lowlevel-8080.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec16.o  \
	../usermem_std-8080.o $(LIBC) end.o
	ack -o fuzix.raw crt0.o devices.o main.o \
	commonmem.o tricks.o v8080.o devtty.o \
	../start.o ../version.o ../lowlevel-8080.o \
	../bankfixed.o ../timer.o ../kdata.o  \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec16.o  \
	../usermem_std-8080.o $(LIBC) end.o
