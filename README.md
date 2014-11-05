FuzixOS: Because Small Is Beautiful

This is the initial public tree for the FuzixOS project. It is not yet useful although you can build and boot it and run
test application code. A lot of work is needed on the utilities and libraries yet.

FUZIX
=====

FUZIX is a fusion of various elements from the assorted UZI forks and
branches beaten together into some kind of semi-coherent platform and then
extended from V7 to somewhere in the SYS3 to SYS5.x world with bits of POSIX
thrown in for good measure. Various learnings and tricks from ELKS and from
OMU also got blended in

What does FUZIX have over UZI
=============================

* Support for multiple processes in banked memory (as per UZI180) but
	with Minix style chmem and efficient use of bank allocations.
* Support for multiple processes via hard disk or non mappable RAM
    drive switching (as per UZI, UZIX).
* The ability to run single tasking on small devices, for bring up
    and for standalone tool execution
* Support for "real" swapping combined with banked memory.
* Proper sane off_t and lseek
* Normal dev_t
* 30 character filenames
* Proper sane time_t
* System 5 signals (half baked)
* Posix termios (does all the original UZI tty did but much can be added)
* Blocking on carrier for terminals
* Optimisations to avoid bogus uarea copying compared to UZI180
* More modern system call API: 3 argument open, mkdir, rmdir, rename,
	chroot (with correct .. semantics), fchdir, fchmod, fchown, fstat,
	fcntl, setpgrp, sighold and friends, waitpid, setpgrp, nice
	O_NDELAY, O_CLOEXEC, F_SETFL, F_DUPFD etc
* Address validation checks on all syscall copies
* Builds with a modern ANSI C compiler (SDCC)
* Core code can be built for 6809 and 6502 so should be far more
	portable
* Core architecture designed to support building and maintaining
	multiple target machines without forking each one
* Helpers to make many bits of implementation wrappers to core code
* Lots more bugs right now

What does UZI have over FUZIX
=============================

* Can run in 64K of RAM (32K kernel/32K user). FUZIX would need
	banked ROM or similar to pull this off.

What do the UZI branches have that FUZIX has not yet integrated
===============================================================

* Minimal TCP/IP (UZIX 2.0). Unfortunately the original TCP was never
released openly.
* Symbolic links (UZIX)
* Various clever fusions of syscalls that may save a few bytes
	(UZIX)
* setprio (UZIX)
* Rather crude loadable drivers (UZIX)
* Use of __naked and __asm for Z80 specific bits to avoid more
	.S files than are needed (UMZIX)

Plus OMU has a really clever function passing trick for open/creat and
friends, while UMZIX has a neat unified "make anything" function.

What Key Features Are Missing Still
===================================
* ptrace, core dumps, ulimit
* root reserved disk blocks
* banked executables
* TCP/IP
* select/poll()
* Z180 banking support
* /dev/tty alias
* Support for > 32MB filesystems (but first figure out how to fsck
	a giant fs on a slow 8bit micro!)
* Uptime
* Smarter scheduler
* Optimisations for disk block/inode allocator (2.11BSD)
* CP/M emulator has not yet been ported to FUZIX syscall API


Tool Issues
===========
* No useful 8086 compiler option (started work on pcc 8086 but help
	needed)
* 6809 gcc and cc65 don't have long long 64bit (for sane time_t)
* SDCC has long long bugs and gaps
* SDCC can generate ROMmable binaries but not banked ones
* SDCC has no register passing function call support, and for some
	stuff it really shows
* SDCC generates quite bloaty small applications. Needs research
	on how to improve.
* None of the above have an O88 style common sequence compressor

Platforms
=========
* Z80Pack - used as a dev and test environment for both large swapping
	mulitprocess and for small single tasking
* Amstrad NC100 - real hardware sanity check

Various other platforms are partly filled out to sanity check assumptions
and start making progress on them. The main need there is now to tackle all
the billion different ways of interfacing the floppy controllers.



