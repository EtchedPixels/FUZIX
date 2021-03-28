CSRCS = libc.c tm4c129x.c devtty.c devsdspi.c devices.c exceptions.c systick.c \
        syscall.c serial.c ssi.c procman.c
ifeq ($(BITS),elf32)
CSRCS += elf.c
endif
ASRCS = tricks.S

GCC_CSRCS = libgcc2.c
GCC_ASRCS = libgcc.S
GCC_COBJS = $(GCC_CSRCS:.c=$(BINEXT))
GCC_AOBJS = $(GCC_ASRCS:.S=.o)

NUTTX_CSRCS = tm4c129_sysctrl.c tiva_lowputc.c tm4c_gpio.c lmxx_tm4c_gpioirq.c \
              arm_modifyreg32.c arm_mdelay.c arm_udelay.c
NUTTX_ASRCS = arm_exception.S
NUTTX_COBJS = $(NUTTX_CSRCS:.c=$(BINEXT))
NUTTX_AOBJS = $(NUTTX_ASRCS:.S=.o)

LCSRCS =
LOBJS = $(LCSRCS:.c=$(BINEXT))

DSRCS = blkdev.c mbr.c devsd.c devsd_discard.c
DOBJS = $(DSRCS:.c=$(BINEXT))

COBJS = $(CSRCS:.c=$(BINEXT))
AOBJS = $(ASRCS:.S=.o)

OBJS  = $(COBJS) $(AOBJS) $(DOBJS) $(LOBJS) \
        $(GCC_AOBJS) $(GCC_COBJS) \
        $(NUTTX_AOBJS) $(NUTTX_COBJS)

CROSS_CCOPTS += -I../dev

all:	crt0.o $(OBJS)

crt0.o: crt0.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(COBJS): %.o: %.c
	$(CROSS_CC) $(CROSS_CCOPTS) -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx/include -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx -c $<

$(AOBJS): %.o: %.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(GCC_COBJS): %.o: from_gcc/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(GCC_AOBJS): %.o: from_gcc/%.S
	$(CROSS_AS) $(ASOPTS) $< -o $*.o

$(NUTTX_COBJS): %.o: from_nuttx/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx/include -c $<

$(NUTTX_AOBJS): %.o: from_nuttx/%.S
	$(CROSS_AS) $(ASOPTS) -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx -I$(ROOT_DIR)/platform-$(TARGET)/from_nuttx/include -D__ASSEMBLY__ $< -o $*.o

$(DOBJS): %.o: ../dev/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

$(LOBJS): %.o: ../lib/%.c
	$(CROSS_CC) $(CROSS_CCOPTS) -c $<

clean:
	rm -f *.o *.elf core* *~

image:
	$(CROSS_LD) --entry=start -nostartfiles -nodefaultlibs \
	-M -o fuzix.elf -T fuzix.ld $(OBJS) \
	../start.o ../version.o ../lowlevel-armm4.o \
	../timer.o ../kdata.o \
	../devio.o ../filesys.o ../process.o ../inode.o ../syscall_fs.o \
	../syscall_proc.o ../syscall_other.o \
	../mm.o ../swap.o ../flat.o \
	../tty.o ../devsys.o ../usermem.o ../syscall_fs2.o \
	../syscall_fs3.o ../syscall_exec$(BITS).o \
	../syscall_level2.o ../level2.o ../select.o \
	../usermem_std-armm4.o ../vt.o ../malloc.o \
	../font8x8.o >../fuzix.map
