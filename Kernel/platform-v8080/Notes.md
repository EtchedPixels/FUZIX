# Virtual 8080 Platform Using Z80Pack

To deal with
- The C compiler bombs on the adventure game builds
- The preprocessor can't handle netd or fforth

Ideally after the 0.4 release replace the C compiler with the Fuzix Compiler
Kit compiler and get it correctly generating 8080 code as well as 8085.

## Memory

We use the default 48K/16K split. We could move to 56K/8K but there seems to
be no particular need.

Memory Map (kernel)

0000-00FF	Interrupt and RST vectors plus helper code
0100-BFFF	Kernel
C000-FFFF	Common and kernel

Memory Map (user)

0000-00FF	Interrupt and RST vectors plus helper code
0100-BDFF	Application
BE00-BFFF	Stashed udata
C000-FFFF	Common and kernel

Put the kernel on the end of a Z80pack floppy image

dd if=fuzix.bin of=drivea.dsk bs=1 seek=193024 conv=notrunc

Write the bootloader to the start of the image

Generate a suitable I drive with

Standalone/build-filesystem drivei.dsk 256 8192

and run cpmsim with the -8 flag for 8080.


Limitations
- The compiler has no soft float library
- The adventure games fail to build due to a compiler bug
- Due to preprocessor limits you cannot build netd or fforth
- uget and uput need optimizing. This shows up significantly in some disk I/O
  cases.
