
CSRCS = devtty.c devrd.c
CSRCS += devices.c main.c libc.c

ASRCS = crt0.s p68hc11.s
ASRCS += tricks.s commonmem.s eeprom.s

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.s=$(BINEXT))
OBJS  = $(COBJS) $(AOBJS)

JUNK = $(CSRCS:.c=.o) $(ASRCS:.s=.o)

all:	$(OBJS) kernel.def eeprom.def

$(COBJS): %$(BINEXT): %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(AOBJS): %$(BINEXT): %.s
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

clean:
	rm -f $(OBJS) $(JUNK)  core *~ 

image:
	$(CROSS_LD) -o fuzix.out -M >../fuzix.map \
	--script=memory.x \
	crt0.o commonmem.o \
	p68hc11.o ../start.o ../version.o ../lowlevel-68hc11.o \
	tricks.o main.o ../timer.o ../kdata.o devrd.o devices.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o ../mm.o ../swap.o ../bankfixed.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o ../syscall_exec16.o \
	../usermem_std-68hc11.o devtty.o libc.o
	$(CROSS_LD) -o eeprom.out -M >../eeprom.map \
	--script=eeprom.x eeprom.o
	m6811-elf-objcopy --output-target=binary fuzix.out ../fuzix.bin
	m6811-elf-objcopy --only-section=.text --only-section=.syscalls --only-section=.vectors --output-target=binary eeprom.out ../eeprom.bin
