
CSRCS = devtty.c
CSRCS += devices.c main.c libc.c

ASRCS = vns32k.S crt0.S
ASRCS += tricks.S

DSRCS =  ../dev/blkdev.c ../dev/mbr.c ../dev/devide.c ../dev/devide_discard.c
DOBJS = $(patsubst ../dev/%.c,%.o, $(DSRCS))

#LSRCS = ../lib/68000exception.c
#LOBJS = $(patsubst ../lib/%.c,%.o, $(LSRCS))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)
OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS)

CROSS_CCOPTS += -I../dev/

JUNK = $(CSRCS:.c=.o) $(ASRCS:.S=.o)

all:	$(OBJS)

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o $(JUNK)  core *~  fuzix.aout

image:
	$(CROSS_LD) -M -o fuzix.aout -n -T fuzix.ld \
	crt0.o \
	vns32k.o ../start.o ../version.o ../lowlevel-ns32k.o \
	tricks.o ../simple.o main.o ../timer.o ../kdata.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec32.o blkdev.o mbr.o \
	devide.o devide_discard.o \
	../usermem_std-ns32k.o devtty.o libc.o ../malloc.o > ../fuzix.map
