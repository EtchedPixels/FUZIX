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
	../inode.c \
	../lowlevel-msp430x.c \
	../kdata.c \
	../process.c \
	../simple.c \
	../start.c \
	../swap.c \
	../syscall_exec16.c \
	../syscall_fs.c \
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

kernel.asflags += \
	-g \

kernel.ldflags += \
	-s 

kernel.result = $(TOP)/kernel-$(PLATFORM).elf
$(call build, kernel, kernel-elf)

