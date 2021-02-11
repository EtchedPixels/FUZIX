How To Build The Fuzix Kernel

This assumes you are running Linux or *BSD or similar. For SDCC at least
this recipe should work on Mac.

- Ensure you have the correct compiler and tools installed

  Z80:
	SDCC 3.5.4 for the kernel (>= #9332 recommended)

	For building banked systems use 9332 with the patches in the Fuzix
	github. For standard kernels patched 9332 or a current SDCC should
	both work. The newest SDCC has the essential parts of the entry size
	patches merged.

	Do not use SDCC 3.7.0 it miscompiles a file. This is fixed in
	r10265

  6502/65C816:
	CC65. Current versions default to fastcall as Fuzix needs and also
	have the \0 misparsing bug fixed.

  6809:
	gcc-4.6.4 for m6809
	lwtools 4.13
	Add (prepend) the directory Build/tools to your PATH environment variable.

  ESP8266:
	xtensa-lx106-elf-gcc (compiler packages are in Debian in the
	gcc-xtensa-lx106 package).

- Set TARGET in the top level Makefile

- For SDCC pick one of the 3 lines with different allocs-per-node. 30,000
  is a good development number, 200000 takes a while but produces better
  code while 1000000 takes hours but produces a fair bit smaller binary

- The path to the SDCC library is guessed by the Makefile but can be set
  by exporting SDCC_LIB to point at the "/lib" directory of SDCC.

- make will build a fuzix.bin, and on some other platforms other files.

- See the platform/README for further instructions on assembling a final
  image. Usually you need to build a file system, dd the kernel into the
  right spot so it occupies the end of the media and then dd the boot block
  on the front.

Kernel Image Notes

The Z80 image is usually built as an SDCC ihx file. SDCC output is intended for
ROM use so the binman tool copies the INITIIALIZER segment to INITIALIZED, then
discards the INITIALIZER. It packs the COMMON segment over the DATA segment and
the FONT segment if present after it. crt0.s for the platform boot then copies
those up to their correct high memory addresses and clears the DATA area.

It doesn't have to work this way. The test px4plus code orders things
differently and binman knows how to handle a few combinations of needs.

--------------------------- Historical README files ------------------------------

       UZI180 - Unix Z80 Implementation for the Z-180 (UZI180)
    Adapted from UZI by Doug Braun and UZI280 by Stefan Nitschke
               Copyright (C) 1998 by Harold F. Bower
                    <HalBower@worldnet.att.net>
          Portions Copyright (C) 1995 by Stefan Nitschke

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License (file LICENSE.TXT) for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

UZI180 is a multi-tasking, multi-user operating for the Zilog Z180 family
of microprocessors.  It is a port of the original UZI incorporating many
fixes from UZI280 and some additional features.  Significant changes
include an updated floppy disk driver based on a NEC765 derivative cont-
roller, a restructuring and reorganization of the various kernel modules
to ease porting to different hardware platforms, a simpler loading
structure from CP/M, and an integral (albeit minimal) CP/M 2.2 emulator.
The task was undertaken to learn more about the Unix operating system and
the C programming language with an eye to using it to replace several
aging CP/M systems.

Like the original UZI, this system is based on Unix Edition 7 which was
one of the direct predecessors of Unix System V.  UZI180 incorporates the
original system calls and the UZI280 extensions added by Stefan Nitschke.
Some minor additions were also added to provide additional System V compa-
tible features.  As stated in the original documentation, there are only a
few significant differences in the UZI kernel calls.  The principal
deviations are:

 - 32-bit return values are not provided, so data structures are passed
   via pointers to pass essential information.  "seek" is therefore incor-
   porated as a kernel function with "lseek" provided as a library call.

 - The system "times" are reported in a different format than the standard
   32-bit count of seconds since 1 January 1970.  32-bit Date/Time struc-
   tures are in the packed binary form used in IBM PCs, therefore
   resulting in 2-second granularity.  As a convention, the 0-99 year
   should be interpreted as: 70-99 implying 1970-1999, and 0-69 being
   2000-2069 to provide Year 2000 compliance.  Standard functions are
   available as library routines.

 - The "stat" call has a different file length format due to the 32-bit
   length omission and use of only 16-bit block and inode numbers.  A data
   structure is used to contain the information (see library source for
   details).

ARCHITECTURE:

UZI180 makes extensive use of the Memory Management (MMU) of the Z180 core
to provide as much process isolation as possible with the chip.  Since the
MMU granularity is 4k, this establishes the basic process structure.  The
top 4k of logical address space (one MMU slice) is defined as Common1 memory
space and contains critical routines that either must always be resident in
memory, or are associated with the currently-running process. Some of the
key elements placed in the Common memory space are:

	- Interrupt Handlers
	- Process Swapper
	- System and Interrupt Stack Space
	- User Process Data Structure
	- Kernel Service Handler

A full 64k is assigned to each process, with 60k available to application
code and data, with the remaining 4k used for system storage.  This
organization permits one 64k bank for the kernel/common memory and seven
process banks on a 512k system where all RAM is usable.  The UZI180 memory
map therefore appears as:

	          First 64k	      Subsequent 64k banks
	FFFF	+------------+		+------------+
    Common	|   Common   |		| Task Store |+
	F000	+------------+		+------------+|+
		|            |		|            |+|+
		|   Kernel   |		|  Process   ||+|
    Banked	|    Code    |		|    Code    |||+
		|            |		|   & Data   ||||
		|            |		|            ||||
	0100	+------------+		+------------+|||
		|  Reserved  |		|  Reserved  |+||
	0000	+------------+		+------------+|+|
					 +------------+|+
					  +------------+|
					   +------------+

Areas marked as "Reserved" are used for Restart and Interrupt vectors,
and to host the required Page 0 storage elements for the CP/M Emulator.
Constructing executable code modules starting at 100H also allows use of
normal CP/M tools without conflicting memory use.

The architecture accomodates MMU base addresses beginning at the first
available RAM address wherever that is.  For YASBEC and P112 systems that
shadow the ROM into RAM, the initial BBR (MMU Base Bank Register) reading
is 08H, while on the MicroMint SB180 RAM begins at 40H.  64k Banks will
therefore begin with BBR values of 08H, 18H, ..., 78H for the former; and
40H, 50H, ..., 70H for a 256k SB180.

For 256k systems that have all RAM available (i.e. do NOT shadow the ROM),
the kernel and only three process banks will be available.  While this may
be minimally usable for a single-user system, a 512k system is the minimum
recommended.  For systems which DO shadow the system ROM, 32k is normally
reserved (beginning at physical RAM address 0), thereby reducing available
RAM by 32k.  This leaves 480k available in a 512k system resulting in only
six process areas after allowing 64k for the kernel and common bank.  The
last 32k will be unused.

The algorithm for computing true 20-bit addresses in a Z180 (for DMA) is:

	BBR	7654 3210
	Addr	     hhhhhhhh llllllll
		----------------------
		bbbb xxxxxxxx yyyyyyyy

An initial check should be made for common area access (assumed to be the
top 4k) which will be addresses of the form "1111hhhh llllllll" or FxxxH.
For these addresses, the raw BBR value for the first 64k of usable RAM is
added.  As an example, the first byte of the Common Area is computed for
DMA use on a P112 with 512k of memory (ROM shadowed) as:

	BBR	0000 1000			08
	Addr	     11110000 00000000		 F000
		----------------------		-----
		0001 01110000 00000000		17000

A kernel variable (osBank) for the Common Area holds the BBR value for the
top 4k which is always accessible by the processor.  It is used to specify
the source address when swapping out, and for the destination address when
swapping in.  The BBR value associated with each process space is stored
both in the kernel process table, and in each process' user data block.
This value is used for the other address in process swaps.

Task switching consists of saving the current process status in the User
stack and data area, block moving the user data area and stacks to the
respective process area in the reserved top 4k (via DMA), restoring the
new task's data into common memory (also via DMA), changing the Bank Base
Register in the Z180 MMU to bring the new task in context, resetting the
new process' stack pointer, restoring processor status and continuing as
before.  This results in relatively rapid response since the bulk of time
required is due to two 768-byte DMA transfers which consume 1536 * 6 T-
states or 9216 / 18,432,000 = .0005 Sec = 500 microseconds on a 18.432 MHz
P112 or YASBEC upgraded with the Z8S180, and double this time, or 1000
microseconds (1 millisecond) on a standard 9.216 MHz YASBEC.  This ignores
any additional time due to insertion of memory wait states and the
relatively insignificant overhead associated with housekeeping.

Time slices were initially set to 50 mS (20 ticks per second) based on one
of the internal Z180 Programmable Reload Timers.  At that rate, less than
4% of a time slice was consumed in swapping processes.  The development
systems have subsequently shortened the slice time to 20 mS to minimize
data loss from polled serial ports.  A disadvantage of shortening the time
is that a greater percentage of CPU time is consumed during process swaps
with a subsequent lengthening of task run times.  This may be adjusted by
appropriately setting parameters in the C and Assembly include files and
re-building the kernel.

Since peripherals in small systems often are not capable of Interrupt-
driven operation, UZI180 implements polling of such ports during timer
interrupt cycles.  Unfortunately, this can lead to inaccurate timekeeping
if the timer is the sole source of Real Time in the system.  A prime
example of this is in the Floppy Disk Driver.  Polled operation often
dictates that interrupts be disabled during disk accesses to avoid data
loss and/or corruption.  During the comparively long time which may elapse
(up to one revolution of 200 milliseconds for 3.5" drives), several clock
ticks may be lost before timekeeping resumes.

Wall-Time is corrected automatically by reading the Real-Time Clock every
time the number of ticks-per-second (set by a constant in include files)
elapses and updating a global kernel variable.  This variable is then used
by kernel functions when necessary to perform such tasks as time stamping
files and returning current time-of-day.

Polled IO sampling on timer interrupts is not so easily handled and data
loss may be significant.  For systems which must operate in this manner,
it is recommended that low-priority processes be placed on those inter-
faces.  Also, use of peripherals that disable interrupts for long periods,
such as polled Floppy Disk drivers, should be minimized.


OPERATION:

The UZI180 kernel is Process 0 and executes in the lowest 64k of RAM in
the system as a standalone CP/M executable program.  Several stages of
initialization occur when first started.  The code and initial data which
is destined for the Common Memory is moved into position, having been
linked to an executable image during the link process.  After initializing
various kernel data elements, Process 1 is initialized in the lower 60k of
the next higher 64k memory region.  Normally Process 1 is "init" which
logs in users and starts (forks) a shell or other user interface.  Each
consecutive fork without termination of a previous process is loaded into
the lower 60k of successively higher 64k increments.

When a new process is initiated, data in the process' address is prepared
with several pieces of data.  Necessary arguments and environment
variables are copied into high addresses along with the pointer arrays per
standard UZI definitions.  Location 0H is initialized with a jump to a
TRAP (illegal instruction fetch) error handler, and location 0030H (RST
30H) with a jump to the kernel service handler.  Both of these handlers
reside in the Common Memory bank beginning at 0F000H.  Version-specific
locations may also be initialized at this phase such as the Interrupt
Vector at 0038H (RST 38) and the Non-Maskable Interrupt at 0066H.

Process swaps are initiated by a simple timeout of a per-process counter
which is decremented.  When a process is initiated (via an execve kernel
call) or the process is swapped into context, the counter is initiated to
a defined priority value.  Each "tick" of the periodic clock interrupt
decrements the counter until it reaches zero.  At that time, the User data
and associated stacks (User, Swap and Kernel) are copied to the highest 4k
of the respective process' memory space via DMA, data for the next
runnable process is loaded into the Common area, and the new process is
swapped into context by remapping the Bank Base value of the Z180's Memory
Management Unit.

UZI180 kernel service calls are initiated by pushing necessary parameters
and the desired function number on the stack and executing a Restart 30H.
A jump to the service handler in common memory was placed at 0030H during
process initialization.  Since a Restart functions as a one-byte CALL, a
return address is placed on the stack below the function number.  The
Service handler obtains the stack address, switches to a kernel stack in
common memory, extracts information (function number and parameters) from
the user stack, switches the kernel in context, and passes control to the
kernel function process handler.  When the function completes (except for
_exit), status information is passed to the kernel exit code in common
memory, the user process is restored to context along with the user stack,
and the Restart "returns" to the user code.  Parameters are pushed onto
the stack in standard C form of right to left as specified in the function
declaration (contrary to UZI and UZI280).  This places the left-most
declared parameter immediately above the return address on the user stack.

Executable programs in UZI display the CP/M roots of the system in that
all programs begin execution at 100H.  In melding the original Unix tech-
nique of testing the first character(s) of a file for "magic numbers" with
CP/M, all executable programs in UZI require that the first instruction be
a long jump (JP xxxx, opcode 0C3H).  For native UZI applications, this
header is contained in the crt.obj file linked with all executable files.
UZI180 modifies this file to follow the jump with a text string containing
the three letters 'UZI'.  If these three letters are missing but the
"magic number" 0C3H is present, then a CP/M executable is assumed and
execution commences under a CP/M emulator described below.


EMULATOR:

In addition to the UZI kernel service calls used by native applications, a
minimal CP/M 2.2 emulator is built into UZI180 to allow many existing CP/M
and Z-System utilities to run under UZI180 without modification.  The
emulator intercepts CP/M Bios and Bdos function requests and interprets
them into equivalent UZI kernel calls, or processes them directly.  To
minimize the amount of memory consumed, only essential functions are
included, and not all programs will function correctly.  Some of the more
significant limitations of the emulator are:

	- No Z-System data structures (ENV, IOP, RCP, FCP) are included
	- Only Drive A:, User 0 (as the current directory) is available
	- No ALV bits are included so Directory programs will be unable
	      to report free space
	- The Bdos size is smaller than 3.5k so programs assuming standard
	      components by subtracting from Bios WB address will be wrong
	- No Command Processor is included

When needed, the emulator is moved into high process memory below the env
and arg parameters.  Instead of transferring execution to the program at
100H as an UZI executable, the process begins by executing the base code
of the emulator.  Code at this location is later overwritten by the CP/M
stack since it is only used during process initialization.  This sequence
sets up CP/M vectors at 0H (Bios Warm Boot), 5H (Bdos service vector),
catenates argv parameters into a capitalized command line tail in the
default buffer starting at 0080H, and initializes other storage locations
(Current Drive/User, FCBs, etc) before beginning execution at 100H.  CP/M
program termination by executing either a Bios Warm Boot or Bdos function
0 will trigger an UZI _exit kernel function and terminate the process.

In the initial form, env parameters are not used by the emulator, but use
of the PATH variable is planned to make the system more usable.


DEVELOPMENT ENVIRONMENT.

The bulk of UZI is programmed in the C language under the CP/M 2.2 or
compatible operating system.  Doug Braun's original UZI was written with
the Codeworks' QC compiler which resulted in some odd interfaces since
parameters were passed the system stack in backwards order (left-to-right)
instead of the conventional right-to-left ordering used in the C program-
ming language.  UZI280 and UZI180 were both written for the Hi-Tech ANSI
Standard compiler Version 3.09 released for non-commercial use in 1987 and
included in Walnut Creek's CP/M CD-ROM.  The Hi-Tech package also included
a Z80/Z180 assembler, linker, librarian and source code for the CP/M run-
time library.  This system forms the complete development environment
(less text editor) for UZI180.

The Hi-Tech Compiler requires a rather large Transient Program Area to
compile the UZI sources.  Consequently, users normally operating with the
Z-System may need to remove several of the standard ZCPR3.x components to
free up enough memory to compile the system.  UZI180 was developed on the
author's Banked and Portable (B/P) Bios system with banked ZSDOS2 and
generated using autosizing of the system (BDOS and ZCPR) with no RCP or
IOP.  Since the compiler overwrites the Command Processor, the base of
BDOS forms the upper limit of Transient Program Area which is 0E486H in
the author's system.  The BIOS Warm Boot vector is at 0EE03H.  This is
only slightly larger than a standard 62k CP/M system where the BDOS vector
is at 0E405H and BIOS WB vector at 0F203H.  A standard 62k CP/M system
should be sufficient since the majority of initial work occured on a
slightly smaller system.

Following Stefan Nitschke's lead with UZI280, assembly-language modules
were added to several portions of kernel code to provide platform-
dependant interfaces where necessary.  Assembly code modules were also
added to improve performance, particularly to minimize context switches
and change parameter passing to the standard 'C' conventions of placing
parameters on the stack from right-to-left.  In some cases, these assembly
modules were necessitated by apparent problems with the Hi-Tech compiler's
ability to handle inline assembly with optimization turned on.  To avoid
severe performance penalties and code bloat, all modules were compiled
with optimization turned ON.

When bringing up UZI180 for the first time, you must first extract all
member files in the basic Kernel library module, and member files for the
specific version for your platform.  Begin by tailoring values in the
version-specific files to reflect your needs, particularly the Hard Disk
parameters in HDCONF.H.  Check the Z180 initialization values in both
Z180ASM.ASZ and ASMDEF.I to insure that such items as Serial Data rates
and CPU clock frequency are accurate.

You must next Compile/Assemble all source code C and Assembly modules for
the kernel.  The simplest way to perform this is to execute the SUBmit
script CMPILUZI.SUB.  In selecting the software tools for this task, use
either the original CP/M 2.2 SUB.COM or an equivalent such as SUPERSUB.
XSUB, EX and ZEX cannot be used since they install in high memory as a
Resident System Extension (RSX) below the Command Processor, thereby
reducing Transient Program Area space and creating problems for the
compiler.  Development of UZI180 used SUPERSUB under the B/P Bios/ZSDOS2
system described earlier.  NOTE: with Operating Systems such as ZSDOS
which do not re-log hard disk partitions on warm boots, the $$$.SUB
temporary file is not re-detected to initiate the next line.  Relogging
of the hard disk can be forced in a ZSDOS system by executing the ZSDOS
utility RELOG.COM after each line of the submit script.  All but the
last line of the script should reflect the appended command when placed
in the Multiple Command Line buffer.  An example compile command line
from CMPILUZI.SUB is:

	c -o -c -v process.c;relog

After each module is compiled/assembled, either manually or via the SUBmit
script, the components must be linked into a loadable kernel image.  A
script is provided for this purpose which is read into the Hi-Tech linker.
The script is set to produce an output file of UZI.COM which appears to
CP/M as a normal COMmand file, but siezes complete control and becomes the
UZI kernel when executed.  This method of using the script with the Hi-
Tech linker is needed to handle the case-sensitivity of segment names in
the Hi-Tech relocatable format.  Invoke the linker redirecting input from
the script file as:

	link <linkuzi

To aid in system maintenance and debugging, a symbol table (UZI.SYM) as
well as the UZI.COM kernel image is also generated in this step.

TOOLKIT.

Doug Braun provided several utilities which allow you to make an UZI file-
system and prepare it for use.  These must be compiled and linked with
modified versions of UZI kernel modules to form CP/M executable programs.
Some of the kernel modules are simply compiled with the utility include
files, while others such as the process and call modules are actually
older versions of the kernel code with some modifications to run as CP/M
programs instead of assuming total control of the computer.  In the case
of the core Floppy Disk driver, XFLOPASM.ASZ, hardware timeout via the
Interrupt timer is removed for utilities in favor of countdown timers and
letting the Floppy Disk motors remain running after the initial spinup.

The basic tools are:

MKFS - Make an UZI file system on a block device (HDx, FDx) and prepare it
        for use.

UDP  - Display specified UZI File System Block Number on the Screen in Hex.

FSCK - Check an UZI file system on a block device for consistency.

UCP  - Copy files between CP/M and an UZI storage device, type files to
        the CP/M Console, mount/umount another device into the UZI file-
        system and delete files.  If you are running ZSDOS with file
        date/time stamping, this program also adds date/time stamps to
        the UZI file system when copying files to the UZI file system.

Portions of the UCP utility were extracted into stand-alone UZI applica-
tions during UZI180 development to test various functions of the kernel,
and to form the necessary core suite of utilities to make the system
"feel" like a Unix system.  These are also provided along with a re-
written LIBC library containing the UZI180 interfaces.  A separate .DOC
file details the steps needed to compile and link applications to form
native UZI180 executables under CP/M.  UCP may then be used to move the
resulting executables into an UZI file system from where they may be
executed.


THINGS TO DO. (Beta Notes, 18 August 1998 - HFB)

- Add complete hardware initialization (probably in MACHASM.ASZ) to set
  all system parameters.  Currently, UZI180 relies on settings performed
  by CP/M or compatible OS/BIOS prior to booting UZI.

- Add TTYASM.ASZ module for setting the TTY port parameters when changed.
  Currently, the serial port parameters (Data rate, # Bits, Parity, etc)
  are whatever was set prior to booting UZI.

- Polish the Floppy Driver (FLOPASM.ASZ) to add variable formats (auto-
  sense on mount?), local bufffering for 1k sector sizes, and accomodate
  disk formats with CP/M Boot Tracks.  The module is currently locked
  into a pseudo-MSDOS 1.44 MB High-Density 3.5" disk format.

- Refine the Emulator a bit and massage the addresses to provide full
  Environment space (512 bytes) and Argument area (512 bytes max).  Also
  add path default to a defined CP/M directory for accessing overlays,
  libraries, etc.


---------------------------------------

    UZIX - UNIX Implementation for MSX
    (c) 1997-2001 Arcady Schekochikhin
	       Adriano C. R. da Cunha
	        http://uzix.msx.org

#What is the UZIX?

UZIX: UNIX IMPLEMENTATION FOR MSX based on UZI written by Douglas
Braun and ported to MS-DOS / MSX by Arcady Schekochikhin and Adriano
Rodrigues da Cunha.

UZIX is an implementation of the UNIX kernel written for a MSX/PC
computer. It implements almost all 7th Edition AT&T UNIX kernel
functionality. UZIX was written to run on PC (under MS DOS) or MSX2/2+/TR.
The source code is written mostly in C, and was compiled with Turbo-C (PC)
or Hitech-C (MSX). UZIX's code was based on public domain Doug Braun's
UZI, which was written from scratch, and contains no AT&T code, so it is
not subject to any of AT&T's copyright or licensing restrictions.

UZIX implements almost all of the 7th Edition functionality. All
file I/O, directories, mountable file systems, user and group IDs, pipes,
and applicable device I/O are supported. The number of processes is
limited only by the swap space available, with a maximum of 31 processes
(total of 1024k memory). UZIX implements UNIX well enough to run the
Bourne Shell in its full functionality.

UZIX is very small. The code size of the kernel is less than 30Kb
(in fact, it's 25.8Kb). I think it is the smallest UNIX 7th Edition-like
implementation you can get.
Due to the limitation of a 64Kb linear addressing space of
Z80, UZIX supports only applications with a maximum size of 32Kb.

Features:

* UZIX is running on any MSX system (MSX2, MSX2+, TR and even emulators);
* Full working multitask environment;
* Full working multiuser environment;
* Stable filesystem, almost bugless;
* Very stable kernel, shell and environment;
* System shell and utilities running with no errors;
* Module support (only used by TCP/IP Stack now);
* Harddisk support (ESE MegaSCSI and Sunrise IDE);

How it works:

UZIX uses MSX2 memory mapper to achieve multiprocessing. On PC
UZIX use additional PC memory for swapping. In both cases UZIX use 64K of
virtual address space (full Z80 space or one full segment on PC).

UZIX itself occupies the upper 32K of address space, and the
currently running process occupies the lower 32K.

UZIX does need some additional hardware support. First, UZIX uses
system clock that provide a periodic interrupt. Also, the current
implementation uses an additional real-time clock to get the time for file
timestamps, etc. The current TTY driver assumes an polling-driven buffered
keyboard, which should exist on most systems.

How UZIX is different than real UNIX:

UZIX implements almost all of the 7th Edition AT&T UNIX
functionality. All file I/O, directories, mountable file systems, user and
group IDs, pipes, and applicable device I/O are supported. Process control
(fork(), execve(), signal(), kill(), pause(), alarm(), and wait()) are
fully supported. The number of processes is limited only by the swap space
available, with a maximum of 31 processes (total of 1024k memory). As
mentioned, UZIX implements UNIX well enough to run the Bourne Shell in its
full functionality. The only changes made to the shell's source code were
to satisfy the limitations of the C compiler.

Here is a (possibly incomplete) list of missing features and
limitations:

* The debugger- and profiler-related system calls do not exist.
* The supplied TTY driver is bare-bones. It supports only one port.
* Inode numbers are only 16-bit, so filesystems are 32MB or less.
* File dates are not in the standard format. Instead they look like those
  used by MS-DOS.
* The 4.2BSD execve() was implemented. Additional flavors of exec() are
  supported by the library.
* The necessary semaphores and locking mechanisms to implement reentrant
  disk I/O are not there. This would make it harder to implement
  interrupt-driven disk I/O without busy-waiting.

Developer notes:

MSX UZIX can be compiled with any ANSI-compatible C compilers. The
only true one for MSX is Hitech-C (CP/M version) and MS-DOS Hitech-C
(cross-compiler). MSX UZIX was written using Hitech-C. You'll find many
constructions and functions not supported (and also limitations) by other
MSX C compilers if you try compiling UZIX with them. Of course UZIX can be
compiled using other compiler, but it will requires a lot of changes in
the source code.  Initially, MSX UZIX couldn't be compiled for running on
a MSX1, since it uses Memory Mapper for multitasking, system real-time
clock for file timestamps, and 80-column screen.
Of course, is possible doing a "light" MSX UZIX for MSX1, with a
fake real-time clock (software emulated by the kernel), using a 40-column
display and other memory device (such as MegaRAM) for multitasking, but
it's not the target of this release.
But, just for fun (and as a curiousity), there is a version of MSX

UZIX for MSX1. It emulates the system real-time clock and uses the
brazilian MegaRAM for multitasking. The system overall performance is
lower than using Memory Mapper, since due to switching restrictions of
MegaRAM (due to UZIX design, MegaRAM pages can be switched only on memory
page 1), some memory block copies are needed for context switching. Also,
the user must input the actual date and time when system boots. The
40-column display doesn't represent a serious restriction, but some
applications (like top, ps or banner) will display bad formatted texts on
screen.

This release of MSX UZIX can handle a maximum of 31 processes
(limited to the size of available RAM). It could handle up to 127
processes (4Mb RAM), but it's nonsense a single user running so many
processes at a time. That's why this limit of 31 concurrent processes.

Known bugs and missing features:

* Some DiskROMs doesn't work well with UZIX. MSX Turbo-R DiskROM also have
  this problem, and it's related to stopping the drive. If the drive stops
  spinning, the next access to it returns a 'not ready' error after a long
  delay. It can be solved with the appropriate settings on 'Advanced
  settings' in UZIX installation program or pressing the space bar while
  ZILO is loading. By an unknown reason, the  drive returns`not ready`
  error for a disk access after stop spinning. The load of init/login
  access disk  continuously, so everything works. During the time UZIX
  presents `login:` and you enter the login name, disk stops spinning.
  The disk access after entering login generates the `not ready` error. The
  solution found is: after each  disk  block  read  access, a reset() is
  done. This is also done in ZILO (via call to 4029h) before loading UZIX
  kernel. The bad side is that this practice slows down disk access a lot
  on MSX machines that aren't Turbo-R.

* Disk access is slowed down since disk buffers were decreased to
  accomodate module functions in kernel. Also, the number of simultaneous
  opened files is less than previous versions (0.1.5 and below);

* TABs are not correctly erased by BS key. DEVTTY backs 8 chars in screen,
  clearing them. It's  not  perfect,  cause the TAB could not  jumped 8
  chars, but less, so we erase chars that are still in stdin queue;

* If lpd can print all the docs, the daemon is fini shed (init  executes
  the  wait()  and  the  daemon exits).If not, the process become zombie
  until SASH exits(init executes the wait() for SASH and lpd). It seems to
  occur only with RuMSX emulator  (sorry, I don't have a printer attached
  to my MSX to test this...).

* UZIX doesn't have the concept of background/foreground applications
  controlled by user;

* Only ESE MegaSCSI and Sunrise IDE harddisk controllers were tested;
  Novaxis SCSI interfaces return error when system tries to access a
  disk; UZIX should run on other interfaces (Bert, Gouda, etc) but
  some fine tunning is necessary;

* With ESE MegaSCSI, stopdrive countdown during interrupt must be
  disabled. If enabled, UZIX can't mount root filesystem. If disabled,
  when accessing  disk-drives, the disk remains spinning forever.

* On UCP, if you have an existing directory (for example, /usr) and want
  to create another dir under this one (for example, /usr/bin), doing:
  cd /,mkdir usr/bin on UCP causes an inconsistent inode count (check
  with fsck). You must do: cd /usr, mkdir bin.

* FILESYS.C sometimes mark fs as bad if no room for writing a file (UCP).

* reset() in DEVFLOP.MSX is a bad procedure. It stops ALL diskdrives
  connected to MSX. A perfect  reset() should stop only the drive given by
  fdrive.

* In passwd.c, getpwuid gets the first password entry that has the given
  UID. It DOESN'T look for GID, so entries with different GIDs and equal
  UIDs will be treated as the same.

Things that would be nice to do:

* Create the man-pages of:
  aal
  chat
  clock
  clone
  cr
  diskusag
  dosdel
  dosemu
  dtree
  find
  finger
  ftp
  gres
  ic
  inodes
  key
  keybstat
  lpd
  lpr
  mailf
  ncheck
  ncr
  netstat
  nslookup
  od
  pathchk
  ping
  pppd
  renice
  roff
  setclock
  setchar
  slattach
  uuencode
  uudecode
* Implement TTY support on curses library. Now all references on code
  were supressed.
* Create a dosformat utility.
* Put a "see also" section on UZIX man-pages.
* Better pipe handling in sash (it only supports two commands now).
* Implement options -c, -l and -s in cmp.
* Create fsck/mkfs/mkboot for UZIX.

Copy license:

	All the UZIX source (kernel, utilities and related sources) is
released under the GNU GPL license. Read the COPYING file for details.

---------------------------------------

		UZI: UNIX Z-80 IMPLEMENTATION

		  Written by Douglas Braun


Introduction:

UZI is an implementation of the Unix kernel written for a Z-80 based
computer.  It implementts almost all of the functionality of the
7th Edition Unix kernel.  UZI was written to run on one specific
collection of custom-built hardware, but since it can easily have device
drivers added to it, and it does not use any memory management hardware,
it should be possible to port it to numerous computers that current use
the CP/M operating system.  The source code is written mostly in C,
and was compiled with The Code Works' Q/C compiler.  UZI's code was
written from scratch, and contains no AT&T code, so it is not subject
to any of AT&T's copyright or licensing restrictions.  Numerous 7th
Edition programs have been ported to UZI with little or no difficulty,
including the complete Bourne shell, ed, sed, dc, cpp, etc.


How it works:

Since there is no standard memory management hardware on 8080-family
computers, UZI uses "total swapping" to achieve multiprocessing.
This has two implications:  First, UZI requires a reasonably fast
hard disk.  Second, there is no point in running a different process
while a process is waiting for disk I/O.  This simplifies the design
of the block device drivers, since they do not have to be interrupt-based.

UZI itself occupies the upper 32K of memory, and the currently running
process occupies the lower 32K.   Since UZI currently barely fits in 32K,
a full 64K of RAM is necessary.

UZI does need some additional hardware support.  First, there must be
some sort of clock or timer that can provide a periodic interrupt.
Also, the current implementation uses an additional real-time clock
to get the time for file timestamps, etc.  The current TTY driver assumes
an interrupt-driven keyboard, which should exist on most systems.
The distribution contains code for hard and floppy disk drivers, but
since these were written for custom hardware, they are provided only
as templates to write new ones.


How UZI is different than real Unix:

UZI implements almost all of the 7th Edition functionality.
All file I/O, directories, mountable file systems, user and group IDs,
pipes, and applicable device I/O are supported.  Process control
(fork(), execve(), signal(), kill(), pause(), alarm(), and wait()) are fully
supported.  The number of processes is limited only by the swap space
available.  As mentioned above,  UZI implements Unix well enough to
run the Bourne shell in its full functionality.  The only changes made
to the shell's source code were to satisfy the limitations of the C compiler.

Here is a (possibly incomplete) list of missing features and limitations:

    The debugger- and profiler-related system calls do not exist.

    The old 6th edition seek() was implemented, instead of lseek().

    The supplied TTY driver is bare-bones.  It supports only one port,
    and most IOCTLs are not supported.

    Inode numbers are only 16-bit, so filesystems are 32 Meg or less.

    File dates are not in the standard format.  Instead they look like
    those used by MS-DOS.

    The 4.2BSD execve() was implemented.  Additional flavors of exec()
    are supported by the library.

    The format of the device driver switch table is unlike that of
    the 7th Edition.

    The necessary semaphores and locking mechanisms to implement
    reentrant disk I/O are not there.  This would make it harder to
    implement interrupt-driven disk I/O without busy-waiting.


A Description of this Release:

Here is a list of the files supplied, and a brief description of each:


intro:		What you are reading

config.h:	Setup parameters, such as table sizes, and the device
		driver switch table.

unix.h:		All strcuture declarations, typedefs and defines.
		(Includes things like errno.h).

extern.h:	Declarations of all global variables and tables.

data.c:		Dummy to source extern.h and devine globals.

dispatch.c:	System call dispatch table.

scall1.c:	System calls, mostly file-related.

scall2.c:	Rest of system calls.

filesys.c:	Routines for managing file system.

process.c:	Routines for process management and context switching.
		Somewhat machine-dependent.

devio.c:	Generic I/O routines, including queue routines.

devtty.c:	Simple TTY driver, slightly-machine dependent.

devwd.c:	Hard disk driver.  Very machine-dependent.

devflop.c:	Floppy disk driver.  Very machine-dependent.

devmisc.c:	Simple device drivers, such as /dev/mem.

machdep.c:	Machine-dependent code, especially real-time-clock and
		interrupt handling code.

extras.c:	Procedures missing from the Q/C compiler's library.

filler.mac:	Dummy to make linker load UZI at correct address.

makeunix.sub:	CP/M SUBMIT file to compile everything.

loadunix.sub:	CP/M SUBMIT file to load everything.


Miscellaneous Notes:

UZI was compiled with the Code Works Q/C C compiler and the Microsoft
M80 assembler under the CP/M operating system, on the same hardware
it runs on.  Also used was a version of cpp ported to CP/M, since
the Q/C compiler does not handle macros with arguments.  However, there
are only a couple of these in the code, and they could easily be removed.

Because UZI occupies the upper 32K of memory, the standard L80 linker
could not be used to link it.  Instead, a homebrew L80 replacement linker
was used.  This generated a 64K-byte CP/M .COM file, which has the lower
32K pruned by the CP/M PIP utility.  This is the reason for appearance
of the string "MOMBASSA" in filler.mac and loadunix.sub.

To boot UZI, a short CP/M program was run that reads in the UZI image,
copies it to the upper 32K of memory, and jumps to its start address.
Other CP/M programs were written to build, inspect, and check UZI filesystems
under CP/M.  These made it possible to have a root file system made before
starting up UZI.  If the demand exists, these programs can be included
in another release.


Running programs under UZI:

A number of 7th Edition, System V, and 4.2BSD programs were ported to
UZI.  Most notably, the Bourne shell and ed run fine under UZI.
In addition the 4.2BSD stdio library was also ported.  This, along
with the Code Works Q/C library and miscellaneous System V library
functions, was used when porting programs.

Due to obvious legal reasons, the source or executables for most of these
programs cannot be released.  However, some kernel-dependent programs
such as ps and fsck were written from scratch and can be included in future
releases.  Also, a package was created that can be linked to CP/M .COM
files that will allow them to run under UZI.  This was used to get
the M80 assembler and L80 linker to run under UZI.  Cpp was also
ported to UZI.  However, it was not possible to fit the Q/C compiler
into 32K, so all programs (and UZI itself) were cross-compiled under CP/M.

The Minix operating system, written for PCs by Andrew Tanenbaum et al,
contains many programs that should compile and run under UZI.  Since
Minix is much less encumbered by licensing provisions than real Unix,
it would make sense to port Minix programs to UZI.  In fact, UZI itself
could be ported to the PC, and used as a replacement for the Minix kernel.
