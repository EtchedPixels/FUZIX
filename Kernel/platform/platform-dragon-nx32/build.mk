$(call find-makefile)

kernelversion.ext = c
kernelversion.srcs = $(abspath $(TOP)/Kernel/makeversion)
$(call build, kernelversion, nop)
$(kernelversion.result):
	@echo MAKEVERSION $@
	$(hide) mkdir -p $(dir $@)
	$(hide) (cd $(dir $@) && $(kernelversion.abssrcs) $(VERSION) $(SUBVERSION))
	$(hide) mv $(dir $@)/version.c $@

# discard segment
kernel.dsrcs = \
	../dev/devide_discard.c \
	../dev/devscsi_discard.c \
	../dev/devsd_discard.c \
	../dev/mbr.c \
	discard.c

kernel.srcs = \
	../dev/blkdev.c \
	../dev/devide.c \
	../dev/devscsi.c \
	../dev/devsd.c \
	../dev/devdw.c \
	../devio.c \
	../devsys.c \
	../filesys.c \
	../inode.c \
	../kdata.c \
	../mm.c \
	../process.c \
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
	../vt.c \
	../font8x8.c \
	../usermem.c \
	../bankfixed.c \
	../lowlevel-6809.s \
	crt0.s \
	dragon.s \
	commonmem.s \
	mem-nx32.s \
	tricks.s \
	usermem_sam.s \
	video.s \
	ide.s \
	scsi_tc3.s \
	floppy.s \
	spi.s \
	drivewire.s \
	devices.c \
	devtty.c \
	devlpr.c \
	devfd.c \
	ttydw.c \
	libc.c \
	main.c \
	$(kernelversion.result)

#FIXME (and also text1/text2)
kernel.srcs += $(kernel.dsrcs)

kernel.includes += \
	-I$(dir $(syscallmap.result)) \
	-I$(TOP)/Kernel/platform-$(PLATFORM)

kernel.cflags += \
	-Wno-int-to-pointer-cast \
	-Wno-pointer-to-int-cast \
	-fno-inline \
	-fno-common \
	-g \

kernel.asflags += \
	-g \

kernel.ldflags += \

kernel.result = $(TOP)/kernel-$(PLATFORM).bin
$(call build, kernel, kernel-elf)

