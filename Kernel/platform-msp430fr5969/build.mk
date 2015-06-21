$(call find-makefile)

kernelversion.ext = c
kernelversion.srcs = $(abspath $(TOP)/Kernel/makeversion)
$(call build, kernelversion, nop)
$(kernelversion.result):
	@echo MAKEVERSION $@
	$(hide) mkdir -p $(dir $@)
	$(hide) (cd $(dir $@) && $(kernelversion.abssrcs) $(VERSION) $(SUBVERSION))
	$(hide) mv $(dir $@)/version.c $@


kernel.srcs = \
	../dev/blkdev.c \
	../dev/devsd.c \
	../dev/devsd_discard.c \
	../dev/mbr.c \
	../devio.c \
	../filesys.c \
	../lowlevel-msp430x.S \
	../kdata.c \
	../process.c \
	../simple.c \
	../start.c \
	../swap.c \
	../timer.c \
	../tty.c \
	../usermem.c \
	crt0.S \
	devices.c \
	devsdspi.c \
	devtty.c \
	libc.c \
	main.c \
	tricks.S \
	$(kernelversion.result)

kernel.cflags += \
	-g \
	-mlarge

kernel.asflags += \
	-g \
	-mlarge

kernel.ldflags += \
	-g 

kernel.libgcc = $(shell $(TARGETCC) -mlarge --print-libgcc)
kernel.result = $(TOP)/kernel-$(PLATFORM).elf
$(call build, kernel, kernel-elf)

