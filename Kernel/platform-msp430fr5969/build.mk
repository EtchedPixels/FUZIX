$(call find-makefile)

kernel.srcs = \
	../devio.c \
	../kdata.c \
	crt0.S \
	devtty.c \
	libc.c \
	$(kernel-seg1.result)

kernel.cflags += \
	-mlarge

kernel.asflags += \
	-mlarge

kernel.libgcc = $(shell $(TARGETCC) -mlarge --print-libgcc)
kernel.result = $(TOP)/kernel-$(PLATFORM).elf
$(call build, kernel, kernel-elf)
		
