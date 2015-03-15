# Don't use this inside recipes; only use it in dependency lists
d := $(HOSTOBJ)/Standalone

$d/chmem: $d/chmem.o
$d/mkfs: $d/mkfs.o $d/util.o
$d/fsck: $d/fsck.o $d/util.o
$d/size: $d/size.o
$d/ucp: $d/ucp.o $d/util.o $d/devio.o $d/xfs1.o $d/xfs1a.o $d/xfs1b.o $d/xfs2.o

# Device nodes to create.
fs_devices = \
	/dev/tty1  20660 513 \
	/dev/tty2  20660 514 \
	/dev/tty3  20660 515 \
	/dev/tty4  20660 516 \
	/dev/tty5  20660 517 \
	/dev/tty6  20660 518 \
	/dev/tty7  20660 519 \
	/dev/tty8  20660 520 \
	/dev/hda   60660 0 \
	/dev/hda1  60660 1 \
	/dev/hda2  60660 2 \
	/dev/hda3  60660 3 \
	/dev/hda4  60660 4 \
	/dev/hda5  60660 5 \
	/dev/hda6  60660 6 \
	/dev/hda7  60660 7 \
	/dev/hda8  60660 8 \
	/dev/hda9  60660 9 \
	/dev/hda10 60660 10 \
	/dev/hda11 60660 11 \
	/dev/hda12 60660 12 \
	/dev/hda13 60660 13 \
	/dev/hda14 60660 14 \
	/dev/hda15 60660 15 \
	/dev/hdb   60660 16 \
	/dev/hdb1  60660 17 \
	/dev/hdb2  60660 18 \
	/dev/hdb3  60660 19 \
	/dev/hdb4  60660 20 \
	/dev/hdb5  60660 21 \
	/dev/hdb6  60660 22 \
	/dev/hdb7  60660 23 \
	/dev/hdb8  60660 24 \
	/dev/hdb9  60660 25 \
	/dev/hdb10 60660 26 \
	/dev/hdb11 60660 27 \
	/dev/hdb12 60660 28 \
	/dev/hdb13 60660 29 \
	/dev/hdb14 60660 30 \
	/dev/hdb15 60660 31 \
	/dev/hdc   60660 32 \
	/dev/hdc1  60660 33 \
	/dev/hdc2  60660 34 \
	/dev/hdc3  60660 35 \
	/dev/hdc4  60660 36 \
	/dev/hdc5  60660 37 \
	/dev/hdc6  60660 38 \
	/dev/hdc7  60660 39 \
	/dev/hdc8  60660 40 \
	/dev/hdc9  60660 41 \
	/dev/hdc10 60660 42 \
	/dev/hdc11 60660 43 \
	/dev/hdc12 60660 44 \
	/dev/hdc13 60660 45 \
	/dev/hdc14 60660 46 \
	/dev/hdc15 60660 47 \
	/dev/hdd   60660 48 \
	/dev/hdd1  60660 49 \
	/dev/hdd2  60660 50 \
	/dev/hdd3  60660 51 \
	/dev/hdd4  60660 52 \
	/dev/hdd5  60660 53 \
	/dev/hdd6  60660 54 \
	/dev/hdd7  60660 55 \
	/dev/hdd8  60660 56 \
	/dev/hdd9  60660 57 \
	/dev/hdd10 60660 58 \
	/dev/hdd11 60660 59 \
	/dev/hdd12 60660 60 \
	/dev/hdd13 60660 61 \
	/dev/hdd14 60660 62 \
	/dev/hdd15 60660 63 \
	/dev/fd0   60660 256 \
	/dev/fd1   60660 257 \
	/dev/fd2   60660 258 \
	/dev/fd3   60660 259 \
	/dev/null  20666 1024 \
	/dev/mem   20660 1025 \
	/dev/zero  20444 1026 \
	/dev/proc  20660 1027 \

fs_files = \
	/usr/lib/liberror.txt   0644 $(TOP)/Standalone/filesystem-src/usr-files/lib/liberror.txt \
	/etc/issue              0644 $(TOP)/Standalone/filesystem-src/etc-files/issue \
	/etc/motd               0644 $(TOP)/Standalone/filesystem-src/etc-files/motd \
	/etc/passwd             0644 $(TOP)/Standalone/filesystem-src/etc-files/passwd \
	/bin/banner             0755 $(OBJ)/Applications/util/banner.exe \
	/bin/basename           0755 $(OBJ)/Applications/util/basename.exe \
	/bin/bd                 0755 $(OBJ)/Applications/util/bd.exe \
	/bin/cal                0755 $(OBJ)/Applications/util/cal.exe \
	/bin/cat                0755 $(OBJ)/Applications/util/cat.exe \
	/bin/chgrp              0755 $(OBJ)/Applications/util/chgrp.exe \
	/bin/chmod              0755 $(OBJ)/Applications/util/chmod.exe \
	/bin/chown              0755 $(OBJ)/Applications/util/chown.exe \
	/bin/cksum              0755 $(OBJ)/Applications/util/cksum.exe \
	/bin/cmp                0755 $(OBJ)/Applications/util/cmp.exe \
	/bin/cp                 0755 $(OBJ)/Applications/util/cp.exe \
	/bin/cut                0755 $(OBJ)/Applications/util/cut.exe \
	/bin/date               0755 $(OBJ)/Applications/util/date.exe \
	/bin/dd                 0755 $(OBJ)/Applications/util/dd.exe \
	/bin/dirname            0755 $(OBJ)/Applications/util/dirname.exe \
	/bin/dosread            0755 $(OBJ)/Applications/util/dosread.exe \
	/bin/du                 0755 $(OBJ)/Applications/util/du.exe \
	/bin/echo               0755 $(OBJ)/Applications/util/echo.exe \
	/bin/factor             0755 $(OBJ)/Applications/util/factor.exe \
	/bin/false              0755 $(OBJ)/Applications/util/false.exe \
	/bin/fdisk              0755 $(OBJ)/Applications/util/fdisk.exe \
	/bin/fgrep              0755 $(OBJ)/Applications/util/fgrep.exe \
	/bin/grep               0755 $(OBJ)/Applications/util/grep.exe \
	/bin/head               0755 $(OBJ)/Applications/util/head.exe \
	/bin/id                 0755 $(OBJ)/Applications/util/id.exe \
	/bin/init               0755 $(OBJ)/Applications/util/init.exe \
	/bin/kill               0755 $(OBJ)/Applications/util/kill.exe \
	/bin/ll                 0755 $(OBJ)/Applications/util/ll.exe \
	/bin/ln                 0755 $(OBJ)/Applications/util/ln.exe \
	/bin/logname            0755 $(OBJ)/Applications/util/logname.exe \
	/bin/ls                 0755 $(OBJ)/Applications/util/ls.exe \
	/bin/mkdir              0755 $(OBJ)/Applications/util/mkdir.exe \
	/bin/mkfs               0755 $(OBJ)/Applications/util/mkfs.exe \
	/bin/mkfifo             0755 $(OBJ)/Applications/util/mkfifo.exe \
	/bin/more               0755 $(OBJ)/Applications/util/more.exe \
	/bin/mv                 0755 $(OBJ)/Applications/util/mv.exe \
	/bin/od                 0755 $(OBJ)/Applications/util/od.exe \
	/bin/pagesize           0755 $(OBJ)/Applications/util/pagesize.exe \
	/bin/passwd             0755 $(OBJ)/Applications/util/passwd.exe \
	/bin/patchcpm           0755 $(OBJ)/Applications/util/patchcpm.exe \
	/bin/printenv           0755 $(OBJ)/Applications/util/printenv.exe \
	/bin/prtroot            0755 $(OBJ)/Applications/util/prtroot.exe \
	/bin/ps                 0755 $(OBJ)/Applications/util/ps.exe \
	/bin/pwd                0755 $(OBJ)/Applications/util/pwd.exe \
	/bin/rm                 0755 $(OBJ)/Applications/util/rm.exe \
	/bin/rmdir              0755 $(OBJ)/Applications/util/rmdir.exe \
	/bin/sleep              0755 $(OBJ)/Applications/util/sleep.exe \
	/bin/sort               0755 $(OBJ)/Applications/util/sort.exe \
	/bin/stty               0755 $(OBJ)/Applications/util/stty.exe \
	/bin/sum                0755 $(OBJ)/Applications/util/sum.exe \
	/bin/su                 0755 $(OBJ)/Applications/util/su.exe \
	/bin/sync               0755 $(OBJ)/Applications/util/sync.exe \
	/bin/tee                0755 $(OBJ)/Applications/util/tee.exe \
	/bin/tail               0755 $(OBJ)/Applications/util/tail.exe \
	/bin/touch              0755 $(OBJ)/Applications/util/touch.exe \
	/bin/tr                 0755 $(OBJ)/Applications/util/tr.exe \
	/bin/true               0755 $(OBJ)/Applications/util/true.exe \
	/bin/uniq               0755 $(OBJ)/Applications/util/uniq.exe \
	/bin/uue                0755 $(OBJ)/Applications/util/uue.exe \
	/bin/wc                 0755 $(OBJ)/Applications/util/wc.exe \
	/bin/which              0755 $(OBJ)/Applications/util/which.exe \
	/bin/who                0755 $(OBJ)/Applications/util/who.exe \
	/bin/whoami             0755 $(OBJ)/Applications/util/whoami.exe \
	/bin/write              0755 $(OBJ)/Applications/util/write.exe \
	/bin/xargs              0755 $(OBJ)/Applications/util/xargs.exe \
	/bin/yes                0755 $(OBJ)/Applications/util/yes.exe \
	/bin/df                 0755 $(OBJ)/Applications/util/df.exe \
	/bin/umount             0755 $(OBJ)/Applications/util/umount.exe \
	/bin/mount              0755 $(OBJ)/Applications/util/mount.exe \
	/bin/fsck               0755 $(OBJ)/Applications/util/fsck.exe \
	/bin/man                0755 $(OBJ)/Applications/util/man.exe \
	/bin/mknod              0755 $(OBJ)/Applications/util/mknod.exe \
	/bin/uud                0755 $(OBJ)/Applications/util/uud.exe \
	/bin/ssh                0755 $(OBJ)/Applications/util/ssh.exe \
	/bin/ed                 0755 $(OBJ)/Applications/util/ed.exe \

# These are bigger than average.
ifneq ($(WANT_LARGE_APPLICATIONS),n)

fs_files += \
	/bin/decomp16           0755 $(OBJ)/Applications/util/decomp16.exe \
	/bin/levee              0755 $(OBJ)/Applications/levee/levee.exe \

endif

# Helper function which returns the source file part of the list above.
get_source_files_from_list = \
	$(if $(strip $1), \
		$(word 3, $1) $(call get_source_files_from_list, $(wordlist 4, 999, $1)), \
		)

$(FILESYSTEM): $(TOP)/Standalone/filesystem-src/populatefs.awk \
		$d/ucp $d/mkfs $d/fsck $(MAKEFILE) \
		$(call get_source_files_from_list, $(fs_files))
	@echo FILESYSTEM $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(HOSTOBJ)/Standalone/mkfs $@ 64 2880
	$(hide) echo $(fs_devices) $(fs_files) \
		| gawk 'BEGIN { RS = " "; } { printf("%s ", $$0); if (n%3 == 2) { printf "\n"; } n++; }' \
		| gawk -f $(TOP)/Standalone/filesystem-src/populatefs.awk \
		| $(HOSTOBJ)/Standalone/ucp $@ > /dev/null
	$(hide) $(HOSTOBJ)/Standalone/fsck $@

install:: $d/chmem $d/mkfs $d/fsck $d/size $d/ucp
	@echo INSTALL standalone tools
	$(hide) mkdir -p $(PREFIX)/bin
	$(hide) cp $^ $(PREFIX)/bin
