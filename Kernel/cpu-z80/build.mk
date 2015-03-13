
# Common flags for all kernel object files.
$(OBJ)/Kernel/%.$O: \
	INCLUDES += \
		-I$(TOP)/Kernel/cpu-$(ARCH) \
		-I$(TOP)/Kernel/platform-$(PLATFORM) \
		-I$(TOP)/Kernel/include

$(OBJ)/Kernel/%.$O: \
	CFLAGS += \
		--stack-auto \
		--constseg CONST

$(OBJ)/Kernel/%.$O: \
	ASFLAGS += \
		-plosff

# Flags for each segment type.
CFLAGS_COMMON = --codeseg COMMONMEM
CFLAGS_SEG1 =
CFLAGS_SEG2 = --codeseg CODE2
# For now but we are overspilling in a lot of configs so will need a real CODE3
CFLAGS_SEG3 = --codeseg CODE2
CFLAGS_SEGDISC = --codeseg DISCARD --constseg DISCARD
CFLAGS_FONT = --constseg FONT
CFLAGS_VIDEO = --codeseg VIDEO

