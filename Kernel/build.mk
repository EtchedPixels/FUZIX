include $(TOP)/Kernel/tools/bankld/build.mk
include $(TOP)/Kernel/tools/build.mk
include $(TOP)/Kernel/cpu-$(ARCH)/build.mk

# Ensure that the platform symlink is set up for before building any
# kernel object.

$(OBJ)/Kernel/%.$O: ASFLAGS += -I$(OBJ)/Kernel
$(OBJ)/Kernel/%.$O: CFLAGS += -I$(OBJ)/Kernel
paths: $(OBJ)/Kernel/platform

$(OBJ)/Kernel/platform:
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
$(OBJ)/Kernel/%.$O: CFLAGS += $(SEGMENT)

# Discardable.

$(OBJ)/Kernel/start.$O \
$(OBJ)/Kernel/dev/devsd_discard.$O: \
	SEGMENT = $(CFLAGS_SEGDISC)

# Segment 1.

$(OBJ)/Kernel/version.$O \
$(OBJ)/Kernel/filesys.$O \
$(OBJ)/Kernel/devio.$O \
$(OBJ)/Kernel/kdata.$O \
$(OBJ)/Kernel/inode.$O \
$(OBJ)/Kernel/tty.$O: \
	SEGMENT = $(CFLAGS_SEG1)

# Segment 2 --- syscalls.

$(OBJ)/Kernel/syscall_proc.$O \
$(OBJ)/Kernel/syscall_fs.$O \
$(OBJ)/Kernel/syscall_fs2.$O \
$(OBJ)/Kernel/syscall_other.$O \
$(OBJ)/Kernel/syscall_exec16.$O \
$(OBJ)/Kernel/syscall_exec32.$O \
$(OBJ)/Kernel/process.$O \
$(OBJ)/Kernel/malloc.$O \
$(OBJ)/Kernel/simple.$O \
$(OBJ)/Kernel/single.$O \
$(OBJ)/Kernel/bank16k.$O \
$(OBJ)/Kernel/bank16k_low.$O \
$(OBJ)/Kernel/bank32k.$O \
$(OBJ)/Kernel/bankfixed.$O \
$(OBJ)/Kernel/flat.$O: \
	SEGMENT = $(CFLAGS_SEG2)

# Drop some bits into CODE3 so the 6502 banks fit nicely. May well
# need to do this on Z80 as well

$(OBJ)/Kernel/devsys.$O \
$(OBJ)/Kernel/mm.$O \
$(OBJ)/Kernel/swap.$O \
$(OBJ)/Kernel/usermem.$O \
$(OBJ)/Kernel/timer.$O: \
	SEGMENT = $(CFLAGS_SEG3)

# Video and font stuff go in their own segments.

$(OBJ)/Kernel/vt.$O: \
	SEGMENT = $(CFLAGS_VIDEO)

$(OBJ)/Kernel/font4x6.$O \
$(OBJ)/Kernel/font6x8.$O \
$(OBJ)/Kernel/font8x8.$O: \
	SEGMENT = $(CFLAGS_FONT)

$(OBJ)/Kernel/version.c: $(TOP)/Kernel/makeversion
	@echo MAKEVERSION $@
	@mkdir -p $(dir $@)
	$(hide) (cd $(dir $@) && $(abspath $<) $(VERSION) $(SUBVERSION) $(PLATFORM))

# For convenience, this lists a bunch of standard kernel objects, to avoid the
# platform having to have a list. This isn't everything; some objects the
# platform has to specifically choose (like the version of syscall_exec), but
# it's the core platform-independent bits of kernel.

KERNEL_OBJS = \
	$(OBJ)/Kernel/devio.$O \
	$(OBJ)/Kernel/devsys.$O \
	$(OBJ)/Kernel/filesys.$O \
	$(OBJ)/Kernel/inode.$O \
	$(OBJ)/Kernel/kdata.$O \
	$(OBJ)/Kernel/malloc.$O \
	$(OBJ)/Kernel/mm.$O \
	$(OBJ)/Kernel/process.$O \
	$(OBJ)/Kernel/start.$O \
	$(OBJ)/Kernel/swap.$O \
	$(OBJ)/Kernel/syscall_fs.$O \
	$(OBJ)/Kernel/syscall_fs2.$O \
	$(OBJ)/Kernel/syscall_other.$O \
	$(OBJ)/Kernel/syscall_proc.$O \
	$(OBJ)/Kernel/timer.$O \
	$(OBJ)/Kernel/tty.$O \
	$(OBJ)/Kernel/usermem.$O \
	$(OBJ)/Kernel/version.$O \

include $(TOP)/Kernel/platform-$(PLATFORM)/build.mk

