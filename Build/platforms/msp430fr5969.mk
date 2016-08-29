$(call find-makefile)

O = o
A = a

# Target gcc setup.

TARGETCC = msp430-elf-gcc
TARGETCPP = msp430-elf-cpp -nostdinc -undef -P
TARGETAS = msp430-elf-as
TARGETAR = msp430-elf-ar
TARGETLD = msp430-elf-ld
TARGETOBJCOPY = msp430-elf-objcopy

targetgcc.cflags += \
	-g \
	-ffunction-sections \
	-fdata-sections \
	-funit-at-a-time \
	-mhwmult=auto \
	-mmcu=msp430fr5969

target-exe.ldflags += \
	-g

targetgcc.includes += -I$(TOP)/Library/include/msp430x

# CPU architecture and which syscall generator to use.

ARCH = msp430x
SYSCALL_GENERATOR = syscall_msp430x
SYSCALL_STUB = fuzixmsp430x/syscall.s
CRT = crt0_msp430x.s

# This is a generic gcc platform.

include $(BUILD)/rules/targetgcc.rules.mk

# Extra, platform-specific libc source files (relative to Library/libs).

libc-functions.localsrcs += \
	setjmp_msp430.s \
	longjmp_msp430.s \

# Configure the filesystem; size and contents. $(FILESYSTEM) lists the files to
# go on the file system, not including the standard files; the three columns
# are destination filename, mode, and source filename.
FILESYSTEM_ISIZE = 640
FILESYSTEM_FSIZE = 20480 # 10MB
FILESYSTEM = \
	/bin/banner             0755 $(util-banner.result) \
	/bin/basename           0755 $(util-basename.result) \
	/bin/bd                 0755 $(util-bd.result) \
	/bin/cal                0755 $(util-cal.result) \
	/bin/cat                0755 $(util-cat.result) \
	/bin/chgrp              0755 $(util-chgrp.result) \
	/bin/chmod              0755 $(util-chmod.result) \
	/bin/chown              0755 $(util-chown.result) \
	/bin/cksum              0755 $(util-cksum.result) \
	/bin/cmp                0755 $(util-cmp.result) \
	/bin/cp                 0755 $(util-cp.result) \
	/bin/cut                0755 $(util-cut.result) \
	/bin/date               0755 $(util-date.result) \
	/bin/dd                 0755 $(util-dd.result) \
	/bin/df                 0755 $(util-df.result) \
	/bin/dirname            0755 $(util-dirname.result) \
	/bin/dosread            0755 $(util-dosread.result) \
	/bin/du                 0755 $(util-du.result) \
	/bin/echo               0755 $(util-echo.result) \
	/bin/ed                 0755 $(util-ed.result) \
	/bin/factor             0755 $(util-factor.result) \
	/bin/false              0755 $(util-false.result) \
	/bin/fdisk              0755 $(util-fdisk.result) \
	/bin/fgrep              0755 $(util-fgrep.result) \
	/bin/fsck               0755 $(util-fsck.result) \
	/bin/grep               0755 $(util-grep.result) \
	/bin/head               0755 $(util-head.result) \
	/bin/id                 0755 $(util-id.result) \
	/bin/kill               0755 $(util-kill.result) \
	/bin/ll                 0755 $(util-ll.result) \
	/bin/ln                 0755 $(util-ln.result) \
	/bin/logname            0755 $(util-logname.result) \
	/bin/ls                 0755 $(util-ls.result) \
	/bin/man                0755 $(util-man.result) \
	/bin/mkdir              0755 $(util-mkdir.result) \
	/bin/mkfifo             0755 $(util-mkfifo.result) \
	/bin/mkfs               0755 $(util-mkfs.result) \
	/bin/mknod              0755 $(util-mknod.result) \
	/bin/more               0755 $(util-more.result) \
	/bin/mount              0755 $(util-mount.result) \
	/bin/mv                 0755 $(util-mv.result) \
	/bin/od                 0755 $(util-od.result) \
	/bin/pagesize           0755 $(util-pagesize.result) \
	/bin/passwd             0755 $(util-passwd.result) \
	/bin/patchcpm           0755 $(util-patchcpm.result) \
	/bin/printenv           0755 $(util-printenv.result) \
	/bin/prtroot            0755 $(util-prtroot.result) \
	/bin/ps                 0755 $(util-ps.result) \
	/bin/pwd                0755 $(util-pwd.result) \
	/bin/rm                 0755 $(util-rm.result) \
	/bin/rmdir              0755 $(util-rmdir.result) \
	/bin/sh                 0755 $(v7-sh.result) \
	/bin/sleep              0755 $(util-sleep.result) \
	/bin/sort               0755 $(util-sort.result) \
	/bin/ssh                0755 $(util-ssh.result) \
	/bin/stty               0755 $(util-stty.result) \
	/bin/su                 0755 $(util-su.result) \
	/bin/sum                0755 $(util-sum.result) \
	/bin/sync               0755 $(util-sync.result) \
	/bin/tail               0755 $(util-tail.result) \
	/bin/tee                0755 $(util-tee.result) \
	/bin/touch              0755 $(util-touch.result) \
	/bin/tr                 0755 $(util-tr.result) \
	/bin/true               0755 $(util-true.result) \
	/bin/umount             0755 $(util-umount.result) \
	/bin/uname              0755 $(util-uname.result) \
	/bin/uniq               0755 $(util-uniq.result) \
	/bin/uud                0755 $(util-uud.result) \
	/bin/uue                0755 $(util-uue.result) \
	/bin/wc                 0755 $(util-wc.result) \
	/bin/which              0755 $(util-which.result) \
	/bin/who                0755 $(util-who.result) \
	/bin/whoami             0755 $(util-whoami.result) \
	/bin/write              0755 $(util-write.result) \
	/bin/xargs              0755 $(util-xargs.result) \
	/bin/yes                0755 $(util-yes.result) \
	/bin/fforth             0755 $(util-fforth.result) \
    /bin/ac                 0755 $(v7-cmd-ac.result) \
    /bin/at                 0755 $(v7-cmd-at.result) \
    /bin/atrun              0755 $(v7-cmd-atrun.result) \
    /bin/col                0755 $(v7-cmd-col.result) \
    /bin/comm               0755 $(v7-cmd-comm.result) \
    /bin/cron               0755 $(v7-cmd-cron.result) \
    /bin/crypt              0755 $(v7-cmd-crypt.result) \
    /bin/dc                 0755 $(v7-cmd-dc.result) \
    /bin/dd                 0755 $(v7-cmd-dd.result) \
    /bin/deroff             0755 $(v7-cmd-deroff.result) \
    /bin/diff               0755 $(v7-cmd-diff.result) \
    /bin/diff3              0755 $(v7-cmd-diff3.result) \
    /bin/diffh              0755 $(v7-cmd-diffh.result) \
    /bin/join               0755 $(v7-cmd-join.result) \
    /bin/makekey            0755 $(v7-cmd-makekey.result) \
    /bin/mesg               0755 $(v7-cmd-mesg.result) \
    /bin/newgrp             0755 $(v7-cmd-newgrp.result) \
    /bin/pr                 0755 $(v7-cmd-pr.result) \
    /bin/ptx                0755 $(v7-cmd-ptx.result) \
    /bin/rev                0755 $(v7-cmd-rev.result) \
    /bin/split              0755 $(v7-cmd-split.result) \
    /bin/su                 0755 $(v7-cmd-su.result) \
    /bin/sum                0755 $(v7-cmd-sum.result) \
    /bin/test               0755 $(v7-cmd-test.result) \
    /bin/time               0755 $(v7-cmd-time.result) \
    /bin/tsort              0755 $(v7-cmd-tsort.result) \
    /bin/wall               0755 $(v7-cmd-wall.result) \
	/init                   0755 $(util-init.result) \
	/usr/games/arithmetic   0755 $(v7-games-arithmetic.result) \
    /usr/games/fish         0755 $(v7-games-fish.result) \
    /usr/games/wump         0755 $(v7-games-wump.result) \
    /usr/games/backgammon   0755 $(v7-games-backgammon.result) \

# These are too big. \

# These don't work yet. \
    /bin/accton             0755 $(Applications/V7/cmd/accton.result) \
    /bin/look               0755 $(Applications/V7/cmd/look.result) \

