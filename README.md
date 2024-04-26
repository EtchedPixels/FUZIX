** ENGINEERING WORK IN PROGRESS **

The Z80 side of the tree is currently moving to the new compiler including
work on the compiler, linker and kernel. Thus some bits of it require you
have absolutely bleeding edge pieces all around. I would suggest avoiding
working on this tree for Z80 stuff right now. Non Z80 should be just fine.

**FuzixOS**: Because Small Is Beautiful

This is the initial public tree for the FuzixOS project. It is not yet useful although you can build and boot it and run
test application code. A lot of work is needed on the utilities and libraries.

# FUZIX

FUZIX is a fusion of various elements from the assorted UZI forks and
branches beaten together into some kind of semi-coherent platform and then
extended from V7 to somewhere in the SYS3 to SYS5.x world with bits of POSIX
thrown in for good measure. Various learnings and tricks from ELKS and from
OMU also got blended in

# Pre-built images

Some pre-built filesystems are now available on www.fuzix.org, and other
images should follow in time.

## Tools

For the 8080, 8085, Z80 and Z180 the code is now built with the Fuzix C
Compiler and Bintools which are also in github. See instructions for
building them. Some kernels still need the customised SDCC 3.8 from from
this github. 65C816 and Z8 are a work in progress moving to this compiler.

6502 is currently built with cc65 and a distribution version should work.

6303/6803 are built with CC6303 (again in this github)

6809 is built with lwtools and the including gcc fork.

Other targets use gcc variants. See the target specific information.

## What does FUZIX have over UZI

* Support for multiple processes in banked memory (as per UZI180) but
	with Minix style chmem and efficient use of bank allocations.
* Support for multiple processes via hard disk or non mappable RAM
    drive switching (as per UZI, UZIX).
* Support for "real" swapping combined with banked memory.
* Proper sane off_t and lseek
* Normal dev_t
* 30 character filenames
* Proper sane time_t
* System 5 signals
* Posix termios (does all the original UZI tty did but much can be added)
* Blocking on carrier for terminals
* Optimisations to avoid bogus uarea copying compared to UZI180
* More modern system call API: 3 argument open, mkdir, rmdir, rename,
	chroot (with correct .. semantics), fchdir, fchmod, fchown, fstat,
	fcntl, setpgrp, sighold and friends, waitpid, setpgrp, nice
	O_NDELAY, O_CLOEXEC, F_SETFL, F_DUPFD etc
* Address validation checks on all syscall copies
* Builds with a modern ANSI C compiler (SDCC)
* Kernel boots to userspace on 6303, 6502, 65C816, 68000, 6803, 6809, 68HC11, 8080, 8085, arm32, esp8266, MSP430 (bitrotted) and eZ80/Z80/Z180
* Core code can be built for 6303, 6502, 65C816, 68000, 6803, 6809, 68HC11, 8080, 8085, 8086, arm32, esp8266, MSP430, pdp11, rabbit r2k/r3k and eZ80/Z80/Z180 so should be far more portable
* Core architecture designed to support building and maintaining
	multiple target machines without forking each one
* Helpers to make many bits of implementation wrappers to core code
* Lots more bugs right now

## What does UZI have over FUZIX

* Can run in 64K of RAM (32K kernel/32K user). FUZIX needs
	banked ROM or similar to pull this off. If you have banked
	ROM then our kernel footprint in RAM is about 8K plus userspace
	plus any framebuffers and similar overhead. On a 6809 it's just
	about possible to run in a straight 64K

## What do the UZI branches have that FUZIX has not yet integrated

* Symbolic links (UZIX)
* Various clever fusions of syscalls that may save a few bytes
	(UZIX)
* setprio (UZIX)
* Rather crude loadable drivers (UZIX)
* Use of __naked and __asm for Z80 specific bits to avoid more
	.S files than are needed (UMZIX)

Plus OMU has a really clever function passing trick for open/creat and
friends, while UMZIX has a neat unified "make anything" function.

## What Key Features Are Missing Still

* ptrace, most of ulimit
* root reserved disk blocks
* banked executables
* TCP/IP (in progress)
* select/poll() (in progress)
* Support for > 32MB filesystems (but first figure out how to fsck
	a giant fs on a slow 8bit micro!)
* Smarter scheduler
* Optimisations for disk block/inode allocator (2.11BSD)

## Tool Issues

* 6809 gcc and cc65 don't have long long 64bit (for sane time_t)
* None of the above have an O88 style common sequence compressor
* CC65 can't handle larger objects on stack, and lacks float support
* We need a 'proper' 65C816 C compiler

[travis-image]: https://travis-ci.org/EtchedPixels/FUZIX.png?branch=master
[travis-url]: https://travis-ci.org/EtchedPixels/FUZIX
