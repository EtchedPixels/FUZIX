$(call find-makefile)

# All filesystems get these.
standard_device_nodes = \
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

FILESYSTEM += \
	/etc/group              0644 Standalone/filesystem-src/etc-files/group \
	/etc/inittab            0644 Standalone/filesystem-src/etc-files/inittab \
	/etc/issue              0644 Standalone/filesystem-src/etc-files/issue \
	/etc/motd               0644 Standalone/filesystem-src/etc-files/motd \
	/etc/passwd             0644 Standalone/filesystem-src/etc-files/passwd \
	/usr/lib/liberror.txt   0644 $(liberror.result) \

# Helper function which returns the source file part of the list above.
get_source_files_from_list = \
	$(if $(strip $1), \
		$(word 3, $1) $(call get_source_files_from_list, $(wordlist 4, 999, $1)), \
		)

filesystem.ext = img
filesystem.abssrcs = $(call get_source_files_from_list, $(FILESYSTEM))
$(call build, filesystem, nop)

$(filesystem.result): $(TOP)/Standalone/filesystem-src/populatefs.awk \
		$(ucp.result) $(mkfs.result) $(fsck.result)
	@echo FILESYSTEM $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $(mkfs.result) $(FILESYSTEM_CROSSENDIAN) $@ $(FILESYSTEM_ISIZE) $(FILESYSTEM_FSIZE)
	$(hide) echo $(standard_device_nodes) $(standard_files) $(FILESYSTEM) \
		| gawk 'BEGIN { RS = " "; } { printf("%s ", $$0); if (n%3 == 2) { printf "\n"; } n++; }' \
		| gawk -f $(TOP)/Standalone/filesystem-src/populatefs.awk \
		| tee /tmp/out \
		| $(ucp.result) $(filesystem.result) > /dev/null
	$(hide) $(fsck.result) $(filesystem.result)


