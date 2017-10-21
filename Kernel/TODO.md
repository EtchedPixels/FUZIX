Big TODO Items Before 0.1 Release
---------------------------------

- [ ]	rename enhancements (directory, overwrite etc) may be L2

- [ ]	ptrace

- [IP]	Core dumps

- [ ]	time_t bits hidden in inode

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

Functionality

- [ ]	remount

- [ ]	config tool


Big Speed Up Points
-------------------

- [ ]	Rewrite the compressor in assembler

- [ ]	Vfork

- [x]	Make mount pin a buffer rather than keeping mount blocks around
	unused.

Maybe
-----
- [ ]	Different magic for "big" fs - 32bit block numbers only on raw
	devices. Split blkno_t into blkno_t blknodev_t or similar

- [ ]	Revoke 8)

- [ ]	Virtual device hooks for networking

- [ ]	Pty/tty devices

- [ ]	Finish select/poll

- [ ]	Support for o65 binary format

Other
-----
- [IP]	Check safety of changes to allow interrupts during swapper

- [x]	Check we don't have any races of the form
	kill()
		checks signal has handler
	signal(x, SIG_DFL)
		signal serviced

	(We now clear the cached signal in this and the mask case. We
	might need to spot 0/-1 sig vector and skip)
