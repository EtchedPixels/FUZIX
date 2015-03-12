include $(TOP)/Kernel/tools/bankld/build.mk
include $(TOP)/Kernel/tools/build.mk

# Shortcuts for use in dependency lists. Don't use these inside recipes
# (the values may have changed by the time they're expanded).
s := $(TOP)/Kernel
o := $(OBJ)/Kernel
t := $(HOSTOBJ)/Kernel/tools
p := platform-$(PLATFORM)

include $s/cpu-$(ARCH)/build.mk

# Ensure that the platform symlink is set up for before building any
# kernel object.

$o/%.$O: ASFLAGS += -I$o
$o/%.$O: CFLAGS += -I$o
paths: $o/platform

$o/platform:
	@echo SYMLINK $@
	@mkdir -p $(dir $@)
	$(hide) ln -sf $(abspath $(TOP)/Kernel/platform-$(PLATFORM)) \
		$(OBJ)/Kernel/platform

# We put these into two code segments so that the caller can plan to
# bank them with a ROM banking tool. We pull out const data because on
# such a system you want to the constants unbanked otherwise badness
# happens on stuff like printf("Hello world\n"); when you bank switch
# the code.
#
# This is also useful if you've got an annoying box where the kernel can't
# be a linear chunk eg if you need to put the kernel in two non adjacent 16K
# chunks or compile it around a hole for videomemory or similar
#
# VT and FONT are handled specially because they can be mapped more
# freely and also because you often want to overlay part of the kernel
# with the framebuffer when doing video output.

SEGMENT = $(error Object file $@ doesn't have a segment specified)
$o/%.$O: CFLAGS += $(SEGMENT)

# Discardable.

$o/start.$O $o/dev/devsd_discard.$O: \
	SEGMENT = $(CFLAGS_SEGDISC)

# Segment 1.

$o/version.$O $o/filesys.$O $o/devio.$O $o/kdata.$O $o/inode.$O $o/tty.$O: \
	SEGMENT = $(CFLAGS_SEG1)

# Segment 2 --- syscalls.

$o/syscall_proc.$O $o/syscall_fs.$O $o/syscall_fs2.$O $o/syscall_other.$O \
$o/syscall_exec16.$O $o/syscall_exec32.$O $o/process.$O $o/malloc.$O \
$o/simple.$O $o/single.$O $o/bank16k.$O $o/bank16k_low.$O $o/bank32k.$O \
$o/bankfixed.$O $o/flat.$O: \
	SEGMENT = $(CFLAGS_SEG2)

# Drop some bits into CODE3 so the 6502 banks fit nicely. May well
# need to do this on Z80 as well

$o/devsys.$O $o/mm.$O $o/swap.$O $o/usermem.$O $o/timer.$O: \
	SEGMENT = $(CFLAGS_SEG3)

# Video and font stuff go in their own segments.

$o/vt.$O: \
	SEGMENT = $(CFLAGS_VIDEO)

$o/font4x6.$O $o/font6x8.$O $o/font8x8.$O: \
	SEGMENT = $(CFLAGS_FONT)

$o/version.c: $s/makeversion
	@echo MAKEVERSION $@
	@mkdir -p $(dir $@)
	$(hide) (cd $(dir $@) && $(abspath $<) $(VERSION) $(SUBVERSION) $(PLATFORM))

# For convenience, this lists a bunch of standard kernel objects, to avoid the
# platform having to have a list. This isn't everything; some objects the
# platform has to specifically choose (like the version of syscall_exec), but
# it's the core platform-independent bits of kernel.

KERNEL_OBJS = \
	$o/devio.$O \
	$o/devsys.$O \
	$o/filesys.$O \
	$o/inode.$O \
	$o/kdata.$O \
	$o/malloc.$O \
	$o/mm.$O \
	$o/process.$O \
	$o/start.$O \
	$o/swap.$O \
	$o/syscall_fs.$O \
	$o/syscall_fs2.$O \
	$o/syscall_other.$O \
	$o/syscall_proc.$O \
	$o/timer.$O \
	$o/tty.$O \
	$o/usermem.$O \
	$o/version.$O \

include $s/$p/build.mk

