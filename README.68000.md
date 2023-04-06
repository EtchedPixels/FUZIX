# 68000 Tool Chain Issues

Some 68000 gcc builds produce something that builds but doesn't quite work.
In particular you see a pattern where it gets to user space but you cannot
login, or it starts init, then without running /etc/rc properly prompts for
a login, tells you it is unable to change the owner of the controlling tty
and crashes.

Do not use any gcc that is built m68k-linux-gnu, these are a weird broken
ucLinux target that doesn't actually work properly, despite being shipped
with Fedora Linux for years

Build yourself a binutils and gcc from the official GNU toolchain. I am
currently using
````
m68k-elf-gcc -v
Using built-in specs.
COLLECT_GCC=m68k-elf-gcc
COLLECT_LTO_WRAPPER=/opt/gcc10-68k/libexec/gcc/m68k-elf/10.3.0/lto-wrapper
Target: m68k-elf
Configured with: ../gcc-10.3.0/configure --target=m68k-elf --prefix=/opt/gcc10-68k --disable-threads --enable-languages=c --disable-shared --without-headers --disable-libquadmath --disable-libssp --disable-libgcj --disable-gold --disable-libmpx --disable-libgomp --disable-libatomic --with-cpu=68000
Thread model: single
Supported LTO compression algorithms: zlib
gcc version 10.3.0 (GCC) 
````

and
````
GNU assembler version 2.37 (m68k-elf) using BFD version (GNU Binutils) 2.37
````

If you think you have this problem try running your kernel with a known good
user space image from fuzix.org or another user. If that works suspect your
compiler.

Some gcc 11 also blow up with

````
/tmp/ccjcB3zX.s: Assembler messages:
/tmp/ccjcB3zX.s:525: Error: value -684 out of range
/tmp/ccjcB3zX.s:525: Error: value of -684 too large for field of 1 byte at 969
````

gcc 12 seems to be ok
