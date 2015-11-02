$(call find-makefile)

kernelversion.ext = c
kernelversion.srcs = $(abspath $(TOP)/Kernel/makeversion)
$(call build, kernelversion, nop)
$(kernelversion.result):
	@echo MAKEVERSION $@
	$(hide) mkdir -p $(dir $@)
	$(hide) (cd $(dir $@) && $(kernelversion.abssrcs) $(VERSION) $(SUBVERSION))
	$(hide) mv $(dir $@)/version.c $@

syscallmap.ext = h
# Do not change the order below without updating main.c.
syscallmap.srcs = \
	../syscall_exec16.c \
	../syscall_fs.c \
	../syscall_fs2.c \
	../syscall_fs3.c \
	../syscall_other.c \
	../syscall_proc.c
$(call build, syscallmap, nop)
$(syscallmap.result): $(map_syscall.result)
	@echo SYSCALLMAP $@
	$(hide) mkdir -p $(dir $@)
	$(hide) $(map_syscall.result) $(syscallmap.abssrcs) > $@
	
$(TOP)/Kernel/platform-msp430fr5969/main.c: $(syscallmap.result)

kernel.srcs = \
	../dev/blkdev.c \
	../dev/devsd.c \
	../dev/devsd_discard.c \
	../dev/mbr.c \
	../devio.c \
	../devsys.c \
	../filesys.c \
	../inode.c \
	../lowlevel-msp430x.c \
	../kdata.c \
	../mm.c \
	../process.c \
	../simple.c \
	../start.c \
	../swap.c \
	../syscall_exec16.c \
	../syscall_fs.c \
	../syscall_fs2.c \
	../syscall_fs3.c \
	../syscall_other.c \
	../syscall_proc.c \
	../timer.c \
	../tty.c \
	../usermem.c \
	crt0.S \
	devices.c \
	devices_discard.c \
	devsdspi.c \
	devsdspi_discard.c \
	devtty.c \
	devtty_discard.c \
	libc.c \
	main.c \
	main_discard.c \
	tricks.S \
	$(kernelversion.result)

kernel.includes += \
	-I$(dir $(syscallmap.result)) \

kernel.cflags += \
	-Wno-int-to-pointer-cast \
	-Wno-pointer-to-int-cast \
	-fno-inline \
	-fno-common \
	-g \

kernel.asflags += \
	-g \

kernel.ldflags += \
	--relax \
	-s 

kernel.result = $(TOP)/kernel-$(PLATFORM).elf
$(call build, kernel, kernel-elf)

