
# Common flags for all kernel object files.
$o/%.$O: \
	INCLUDES += \
		-I$s/cpu-$(ARCH) \
		-I$s/platform-$(PLATFORM) \
		-I$s/include

$o/%.$O: \
	CFLAGS += \
		--stack-auto \
		--constseg CONST

$o/%.$O: \
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

