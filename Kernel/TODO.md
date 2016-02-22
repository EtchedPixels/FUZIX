Big TODO Items Before 0.1 Release
---------------------------------

- [x] BSD groups can be done but do we care ? (yes but Level 2 only)

- [x]	Termios and speed hooks to tty drivers

- [x]	hangup ioctl (vhangup) plus hangups for group leader exits

- [x]	rename should delete old files it renames over

- [x]	SYS5 signal functionality and other signals (SIGCLD, STOP etc)

- [x]	SYS5 signal holding

- [x]	ptrace

- [IP]	Core dumps

- [ ]	time_t bits hidden in inode

- NA	RTC setting (do in user space)

- [x]	Lock clock to RTC seconds

- [ ]	Can we make the mount point buffers writable to disk too so we can
	drop the quiet ones when busy ?

- IP	Simplify exec logic and split into multiple functions

- [ ]	Add "shared lib" (or more accurately copied lib) support for libc
	to keep binary size small

- [ ]	Add an exomizer 2.0 linking mode to save disc space. Probably
	can be done all user space.

- [ ]	Can we make inodes partially pageable given our on disk guarantees ?
	Disk inode in cinode would then become a pointer. Might allow more open
	objects and less memory usage. Might be nicer alternative to the BSD
	inode shrinking hack (although that would fix the time_t question!)

- [ ]	Finish the cpm emulator port

- IP	Make object alignments 16bit friendly

- [x]	Add fields to binaries giving load page (so can load Z80 0x8000
	binaries safely etc)

Big Speed Up Points
-------------------

- [ ]	Rewrite the compressor in assembler

- [x]	Support 'raw' I/O on files	(done for O_DIRECT read and as an
	optimisation). Needs cleaning up.

- [x]	Make execve use this to avoid all the copies

- [ ]	Vfork

- [x]	Make mount pin a buffer rather than keeping mount blocks around
	unused.

Maybe
-----
- [ ]	Different magic for "big" fs - 32bit block numbers only on raw
	devices. Split blkno_t into blkno_t blknodev_t or similar

- [x]	Carrier handling for tty devices

- [ ]	Revoke 8)

- [x]	Uptime/loadaverage/free

- [ ]	swapfree

- [ ]	Virtual device hooks for networking

- [ ]	Pty/tty devices

- [ ]	Finish select/poll

- [ ]	Support for o65 binary format

Other
-----
- [ ]	Check safety of changes to allow interrupts during swapper

- [ ]	Check we don't have any races of the form
	kill()
		checks signal has handler
	signal(x, SIG_DFL)
		signal serviced
