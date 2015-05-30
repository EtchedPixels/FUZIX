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

targetgcc.includes += -I$(TOP)/Library/include/msp430x

# CPU architecture and which syscall generator to use.

ARCH = msp430x
SYSCALL_GENERATOR = syscall_msp430x
SYSCALL_STUB = fuzixmsp430x/syscall.s
CRT = crt0_msp430x.s

# This is a generic gcc platform.

include $(BUILD)/targetgcc.rules.mk


# Configure the filesystem; size and contents. $(FILESYSTEM) lists the files to
# go on the file system, not including the standard files; the three columns
# are destination filename, mode, and source filename.
FILESYSTEM_ISIZE = 64
FILESYSTEM_FSIZE = 2880
FILESYSTEM = \
	/bin/arithmetic         0755 $(Applications/V7/games/arithmetic.result) \
	/bin/banner             0755 $(Applications/util/banner.result) \
	/bin/basename           0755 $(Applications/util/basename.result) \
	/bin/bd                 0755 $(Applications/util/bd.result) \
	/bin/cal                0755 $(Applications/util/cal.result) \
	/bin/cat                0755 $(Applications/util/cat.result) \
	/bin/chgrp              0755 $(Applications/util/chgrp.result) \
	/bin/chmod              0755 $(Applications/util/chmod.result) \
	/bin/chown              0755 $(Applications/util/chown.result) \
	/bin/cksum              0755 $(Applications/util/cksum.result) \
	/bin/cmp                0755 $(Applications/util/cmp.result) \
	/bin/cp                 0755 $(Applications/util/cp.result) \
	/bin/cut                0755 $(Applications/util/cut.result) \
	/bin/date               0755 $(Applications/util/date.result) \
	/bin/dd                 0755 $(Applications/util/dd.result) \
	/bin/df                 0755 $(Applications/util/df.result) \
	/bin/dirname            0755 $(Applications/util/dirname.result) \
	/bin/dosread            0755 $(Applications/util/dosread.result) \
	/bin/du                 0755 $(Applications/util/du.result) \
	/bin/echo               0755 $(Applications/util/echo.result) \
	/bin/ed                 0755 $(Applications/util/ed.result) \
	/bin/factor             0755 $(Applications/util/factor.result) \
	/bin/false              0755 $(Applications/util/false.result) \
	/bin/fdisk              0755 $(Applications/util/fdisk.result) \
	/bin/fgrep              0755 $(Applications/util/fgrep.result) \
	/bin/fsck               0755 $(Applications/util/fsck.result) \
	/bin/grep               0755 $(Applications/util/grep.result) \
	/bin/head               0755 $(Applications/util/head.result) \
	/bin/id                 0755 $(Applications/util/id.result) \
	/bin/init               0755 $(Applications/util/init.result) \
	/bin/kill               0755 $(Applications/util/kill.result) \
	/bin/ll                 0755 $(Applications/util/ll.result) \
	/bin/ln                 0755 $(Applications/util/ln.result) \
	/bin/logname            0755 $(Applications/util/logname.result) \
	/bin/ls                 0755 $(Applications/util/ls.result) \
	/bin/man                0755 $(Applications/util/man.result) \
	/bin/mkdir              0755 $(Applications/util/mkdir.result) \
	/bin/mkfifo             0755 $(Applications/util/mkfifo.result) \
	/bin/mkfs               0755 $(Applications/util/mkfs.result) \
	/bin/mknod              0755 $(Applications/util/mknod.result) \
	/bin/more               0755 $(Applications/util/more.result) \
	/bin/mount              0755 $(Applications/util/mount.result) \
	/bin/mv                 0755 $(Applications/util/mv.result) \
	/bin/od                 0755 $(Applications/util/od.result) \
	/bin/pagesize           0755 $(Applications/util/pagesize.result) \
	/bin/passwd             0755 $(Applications/util/passwd.result) \
	/bin/patchcpm           0755 $(Applications/util/patchcpm.result) \
	/bin/printenv           0755 $(Applications/util/printenv.result) \
	/bin/prtroot            0755 $(Applications/util/prtroot.result) \
	/bin/ps                 0755 $(Applications/util/ps.result) \
	/bin/pwd                0755 $(Applications/util/pwd.result) \
	/bin/rm                 0755 $(Applications/util/rm.result) \
	/bin/rmdir              0755 $(Applications/util/rmdir.result) \
	/bin/sleep              0755 $(Applications/util/sleep.result) \
	/bin/sort               0755 $(Applications/util/sort.result) \
	/bin/ssh                0755 $(Applications/util/ssh.result) \
	/bin/stty               0755 $(Applications/util/stty.result) \
	/bin/su                 0755 $(Applications/util/su.result) \
	/bin/sum                0755 $(Applications/util/sum.result) \
	/bin/sync               0755 $(Applications/util/sync.result) \
	/bin/tail               0755 $(Applications/util/tail.result) \
	/bin/tee                0755 $(Applications/util/tee.result) \
	/bin/touch              0755 $(Applications/util/touch.result) \
	/bin/tr                 0755 $(Applications/util/tr.result) \
	/bin/true               0755 $(Applications/util/true.result) \
	/bin/umount             0755 $(Applications/util/umount.result) \
	/bin/uniq               0755 $(Applications/util/uniq.result) \
	/bin/uud                0755 $(Applications/util/uud.result) \
	/bin/uue                0755 $(Applications/util/uue.result) \
	/bin/wc                 0755 $(Applications/util/wc.result) \
	/bin/which              0755 $(Applications/util/which.result) \
	/bin/who                0755 $(Applications/util/who.result) \
	/bin/whoami             0755 $(Applications/util/whoami.result) \
	/bin/write              0755 $(Applications/util/write.result) \
	/bin/xargs              0755 $(Applications/util/xargs.result) \
	/bin/yes                0755 $(Applications/util/yes.result) \
    /bin/ac                 0755 $(Applications/V7/cmd/ac.result) \
    /bin/at                 0755 $(Applications/V7/cmd/at.result) \
    /bin/atrun              0755 $(Applications/V7/cmd/atrun.result) \
    /bin/col                0755 $(Applications/V7/cmd/col.result) \
    /bin/comm               0755 $(Applications/V7/cmd/comm.result) \
    /bin/cron               0755 $(Applications/V7/cmd/cron.result) \
    /bin/crypt              0755 $(Applications/V7/cmd/crypt.result) \
    /bin/dc                 0755 $(Applications/V7/cmd/dc.result) \
    /bin/dd                 0755 $(Applications/V7/cmd/dd.result) \
    /bin/deroff             0755 $(Applications/V7/cmd/deroff.result) \
    /bin/diff               0755 $(Applications/V7/cmd/diff.result) \
    /bin/diff3              0755 $(Applications/V7/cmd/diff3.result) \
    /bin/diffh              0755 $(Applications/V7/cmd/diffh.result) \
    /bin/join               0755 $(Applications/V7/cmd/join.result) \
    /bin/makekey            0755 $(Applications/V7/cmd/makekey.result) \
    /bin/mesg               0755 $(Applications/V7/cmd/mesg.result) \
    /bin/newgrp             0755 $(Applications/V7/cmd/newgrp.result) \
    /bin/pr                 0755 $(Applications/V7/cmd/pr.result) \
    /bin/ptx                0755 $(Applications/V7/cmd/ptx.result) \
    /bin/rev                0755 $(Applications/V7/cmd/rev.result) \
    /bin/split              0755 $(Applications/V7/cmd/split.result) \
    /bin/su                 0755 $(Applications/V7/cmd/su.result) \
    /bin/sum                0755 $(Applications/V7/cmd/sum.result) \
    /bin/test               0755 $(Applications/V7/cmd/test.result) \
    /bin/time               0755 $(Applications/V7/cmd/time.result) \
    /bin/tsort              0755 $(Applications/V7/cmd/tsort.result) \
    /bin/wall               0755 $(Applications/V7/cmd/wall.result) \
    /usr/games/backgammon   0755 $(Applications/V7/games/backgammon.result) \
    /usr/games/fish         0755 $(Applications/V7/games/fish.result) \
    /usr/games/wump         0755 $(Applications/V7/games/wump.result) \

# These don't work yet. \
    /bin/accton             0755 $(Applications/V7/cmd/accton.result) \
    /bin/look               0755 $(Applications/V7/cmd/look.result) \

