$(call find-makefile)

O = rel
A = lib

# CPU architecture and which syscall generator to use.

ARCH = z80
SYSCALL_GENERATOR = syscall
SYSCALL_STUB = fuzix/syscall-zx128.s
CRT = crt0-zx128.s

# This platform uses sdcc.

include $(BUILD)/rules/sdcc.rules.mk

# Extra, platform-specific libc source files (relative to Library/libs).

libc-functions.localsrcs += \
	setjmp.c \

# Configure the filesystem; size and contents. $(FILESYSTEM) lists the files to
# go on the file system, not including the standard files; the three columns
# are destination filename, mode, and source filename.
FILESYSTEM_ISIZE = 64

# for 85k Microdrive cartridge
#FILESYSTEM_FSIZE = 168

# for 655350 TR-DOS disk
FILESYSTEM_FSIZE = 1280

FILESYSTEM = \
	/bin/cat                0755 $(util-cat.result) \
	/bin/cp                 0755 $(util-cp.result) \
	/bin/echo               0755 $(util-echo.result) \
	/init                   0755 $(util-init.result) \
	/bin/ls                 0755 $(util-ls.result) \
	/bin/mkdir              0755 $(util-mkdir.result) \
	/bin/mv                 0755 $(util-mv.result) \
	/bin/printenv           0755 $(util-printenv.result) \
	/bin/ps                 0755 $(util-ps.result) \
	/bin/pwd                0755 $(util-pwd.result) \
	/bin/rm                 0755 $(util-rm.result) \
	/bin/rmdir              0755 $(util-rmdir.result) \
	/bin/sleep              0755 $(util-sleep.result) \
	/bin/ssh                0755 $(util-ssh.result) \
	/bin/touch              0755 $(util-touch.result) \
	/bin/who                0755 $(util-who.result) \
	/bin/whoami             0755 $(util-whoami.result) \
