$(call find-makefile)

kernel-seg1.srcs = \
	libc.c \
	devtty.c \
	../devio.c \
	../kdata.c \

kernel-seg1.cflags += -I$(TOP)/Kernel
$(call build, kernel-seg1, kernel-lib)

kernel.srcs = \
	crt0.S \
	$(kernel-seg1.result)
kernel.result = $(TOP)/kernel-$(PLATFORM).elf
$(call build, kernel, kernel-elf)
		
