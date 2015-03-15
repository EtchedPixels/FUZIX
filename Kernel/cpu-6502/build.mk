
# Common flags for all kernel object files.
$(OBJ)/Kernel/%.$O: \
	private INCLUDES += \
		-I$(TOP)/Kernel/cpu-$(ARCH) \
		-I$(TOP)/Kernel/platform-$(PLATFORM) \
		-I$(TOP)/Kernel/include

$(OBJ)/Kernel/%.$O: \
	private CFLAGS += \

$(OBJ)/Kernel/%.$O: \
	private ASFLAGS += \

# Flags for each segment type.
CFLAGS_COMMON = --code-name COMMONMEM
CFLAGS_SEG1 = --code-name SEG1
CFLAGS_SEG2 = --code-name SEG2
CFLAGS_SEG3 = --code-name SEG3
CFLAGS_SEGDISC = --code-name DISCARD --rodata-name DISCARDDATA
CFLAGS_FONT = --code-name SEG3
CFLAGS_VIDEO = --code-name SEG3
