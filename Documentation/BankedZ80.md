# Porting Fuzix To A Banked Z80 Machine

## Introduction

Z80 based systems were typically built with a fixed common memory space at
the top of the address space and multiple banks below that threshold. For
such machines Fuzix provides all the basic management and switching support
code needed.

This approach can also be useful for machines with more complicated memory
paging arrangements such as multiple 16K pages and while less efficient in
memory usage is a convenient way to bring the system up initially.

## Reference Platform

The reference platform for this document is the Microbee platform
Kernel/platform-ubee. This platform is a real machine and shows both the
use of standard banking code, the vt interface for systems with a real
keyboard and display built in, and how to handle screen maps.

It is less typical in that it has 32K of common and 32K of bankable memory.

For Z180 platforms please see the Z180 documentation instead. As the Z180
has a standardised MMU the port is even easier.

## Configuration Files

Your platform needs a minimum of the following files

- Makefile
- kernel.def
- config.h
- target.mk

The recommended approach is to copy these from an existing port and then
edit them as neccessary.

For an initial banked memory platform edit config.h and change the following
parameters

CONFIG_RTC: set this if you have a real time clock which can provide a
current seconds value when read. Fuzix uses this to lock the system clock
against a time source even when interrupt disables may cause missed ticks.

CONFIG_VT: this and the corresponding VT_ paramemters should be set if you
have a display console. If you have a serial port you can use it is usually
a lot easier to bring the platform up on the serial port first.

TICKSPERSEC: set this to the number of ticks per second your timer interrupt
provides. It should be a multiple of 10. If you have only fast timers you
can use a counter in your interrupt handler and only invoke the Fuzix
interrupts when needed.

FIXME: BANKS

PROGBASE: this is the base address of programs. On all normal Z80 systems
this will be zero.
PROGLOAD: is the load address of programs. Like CP/M Fuzix uses 0x0100 in
order to leave the exception vectors free.
PROGTOP: needs to be set to 768 bytes below the common memory base. The
bytes above this are used by the kernel to store per process data
PROC_SIZE: the number of KB required per process. This will usually be the
size of the bank region

Delete any swap configuration if your machine has enough banks to hold the
kernel and 3 processes. If you have less you may need to implement swap from
the beginning.

To begin with set BOOT_TTY to (512 + 1), set NUM_DEV_TTY to 1 and implement
just the serial port for bring up.

kernel.def is the matching assembly language definition file.

U_DATA: this should be the base of your common space

U_DATA_STASH: should match PROGTOP in config.h

PROGBASE and PROGLOAD: these should match config.h

Z80_TYPE: Should be set to 0 for a CMOS Z80, or 1 for an NMOS Z80. If you
are unsure set the value to 1. This will then compile in workarounds for the
hardware bugs in the NMOS Z80 processor.

CONFIG_SWAP: should be zero for now

NBUFS: should match NBUFS in the kernel.

## Memory Map

### Kernel Map

The initial segment the kernel runs from plus common memory is laid out in
the following order:

Vectors: Low 0x100 bytes. Except for the needed RST instructions the rest of
this space can be used and some platforms even load the kernel lower. For
CP/M platforms however 0x100 makes an easy base and means the kernel can be
loaded as a .COM file and initially even debugged under DDT.

Code: Two banks of code starting at 0x100 and continuing upwards along with
constants

Data: Mostly zero. This holds the various static variables and data used by
the kernel, along with arrays for process, file system and the like.

Buffers: This is the final part of the main kernel image. It holds the
initial buffers for the disk cache.

Discard: Code/Data that is executed only during bring up. This is discarded
at the point the init user process load begins and the space is reclaimed as
disk cache buffers. The buffer space reclain continues up until the base of
the kernel common area.

Common: This holds code and data that must remain mapped at all times,
including bank switching code, functions that copy memory to user banks and
the data transfer routines for disk I/O. If you are familiar with CP/M 3
common memory the approach is similar except that less needs to be in
common space. The common space for the banked model starts with 768 bytes of
udata and stack. This holds data specific to the current process that can be
swapped out with that process, and which is only needed when that process is
running.

### User Map

The user space memory map is in the following order

Vectors: Low 0x100 bytes. On most platforms this space is available to use
except for the system call (RST 38h) and interrupt vectors. It should only
be used by the CP/M emulator for portability reasons.

Code: The binary of the executing process is loaded at 0x100 and executed
from that address.

Data: This follows the code and consists of the initialized data of the
current process.

BSS: Zeroed space. It begins at the size specified in the executable header
but can be extended via the brk() and sbrk() system calls.

Space: Unclaimed space between the top of the BSS and the bottom of the
stack. This space should not be accessed by a process.

Stack: From the current stack pointer up to the top of the user process
memory. On entry this holds arguments and environment. It is used for the
process stack throughout execution.

Save Area: The top 768 bytes of the per process memory map hold a saved copy
of the udata when that process is not executing.

Above this is the common area of the kernel map.

### Dealing With Awkward Common Sizes

There are two cases that are found. Many systems (particularly those aimed
at MP/M) have a common area of 16K or more. For these platforms the user
process space is limited to the size of the non-common area and the kernel
non-common area may continue into common space, with the 'common' as the
kernel sees it set much higher. No special handling of this is needed, just
set Fuzix up as if the common memory was somewhere higher up

The second case is where the common memory is too small. In these cases
place the common data in the real common and if need be copy the remaining
common code into the top of each bank at boot time so that whatever bank is
selected the common code is present.

The final case covers systems such as the Cromenco where there is no common
memory as such, instead a mechanism exists to copy the same code or data
into multiple banks at once. For these systems the same approach can be used
as when the common is too small. However a great deal of additional case is
needed because variables in common memory will not update all the banks.

## Standard Files

There are two standard files that are needed for the system and provided by
generic code for all banked Z80 boxes. The commonmem.s and tricks.s files
include standard support code and should be taken as-is from an existing
banked Z80 port.

## Low Level Assembly Functions

These can be found in the file ubee.s, and similar functions can be found in
other banked ports to give more examples (eg trs80, mtx, z80pack).

The assembly level functions are invoked by the low level support code for
the processor. This can be read at lowlevel-z80.s and the files it includes.

Note that SDCC starts all C symbols with a hidden _ so that other assembly
labels are not visible in the C namespace.

### Start up code

The first code linked is crt0.s. This can be customized per platform in
order to do the initial setup needed. It is handy to link it to 0x0100 and
execute from this address as it can then be executed from CP/M.

The start up code needs to disable interrupts, and set up a valid stack
pointer. Once this is done it invokes init_early for any platform specific
early initialization.

After this returns  it needs to relocate common and discard memory. When the
kernel is built the image is packed for size with the common and discard placed
over the zero data area. This makes the image smaller and also allows it to
load in situations where it must take some action before using the higher
memory.

The crt0 code needs to copy it to the correct location and
then wipe the data segment. Some platforms may want to do early checks here
(for example before wiping out CP/M) so they can return nicely to the boot
environment when an error occurs.

After copying the banks into place and zeroing memory the startup should set
the stack to kstack_top (in the common space) if it did not already do so,
and then call init_hardware to do early hardware initialization. Finally it
invokes_fuzix_main which enters the C environment and never returns.

platform-z80pack provides an example of a fairly minimal crt0.s

### The following should be placed in common memory (.area _COMMONMEM)

_platform_monitor: called when something bad happens. This can either spin
or can invoke a monitor if one is loaded. If a monitor is not present in ROM
then One option is to reserve another bank for a monitor/debugger and switch
bank. Another option is to write the memory image to disk.

_platform_reboot: called when the user shuts down and exits Fuzix. This
ideally will switch the memory map back and invoke any boot rom. On some
platforms this is not possible so it may just need to spin.

_program_vectors: this is called from C when a bank is being set up. It is
responsible for setting up the entry points in the low memory space. It is
also invoked with the parameter NULL during boot to initialize the vectors
in the kernel space. For most platforms this should be identical.

platform_interrupt_all: is invoked at the start of the kernel interrupt
handler being called. It provides a hook for any special processing but can
normally just be a ret instruction.

map_kernel: set the bank mapping up so that the kernel is mapped into
memory. This routine should save and restore any registers it uses.

map_process: map the process whose bank number is in (HL), or if NULL map
the kernel. This function may corrupt HL and AF but no other registers.

map_process_a: map the process whose bank number is in the A register. This
should corrupt no other registers except AF. Usually it can be implemented
as part of map_process.

map_process_always: map the current process. This must not corrupt any
registers. On most platforms it can be implemented as

	push af
	push hl
	ld hl,#U_DATA__U_PAGE
	call _map_process
	pop hl
	pop af
	ret

but in some cases there may be more efficient methods to do this.

map_save: saves a copy of the current mapping. If any additional banking is
being done within the kernel (such as video or ROM overlay) it should
correctly record the state. This may mean that map_process and friends need
to record the state they have set up, especially if the bank registers are
write only. For banked Z80 it is guaranteed that map_save will be followed
by map_kernel. It is also guaranteed that it is safe to call map_kernel from
within map_save, providing the map saved is that before map_kernel is
invoked. This is not normally necessary but if you are executing on a
platform with no true common it will be, in order to save the state where
map_restore can find it.

map_restore: restore the mapping saved in the map_save call. Because systems
with no real common have no banked data memory to store the maps safely it
is guaranteed that the map_restore will be called with the kernel bank
mapped.

outchar: this is used by asm debug routines and it should output the
character in A to whatever port is being used for debug. For a machine with
serial ports this may well be the serial console itself.

### The following routine must be in code (.area _CODE) memory space.

init_early: this function is invoked during the initial boot up of the
system before any C code is executed and before the common and disard is
setup and available. This routine should only do whatever must be done
before the memory is set up properly.

### The following routine may be in code or discard (.area _DISCARD) memory space.

init_hardware: is invoked before the C entry. It is responsible for setting
the memory size in Kb of the system and placing it in _ramsize, as well as
placing the memory size ignoring the kernel in _procmem. For a banked system
the total memory is normally the size of the real non common area multipled
by the number of banks plus the size of the common, and the size of process
memory is 64K less. init_hardware should also invoke _program_vectors with a
argument on the stack of 0.

## Low Level Data

### Data in buffers (.area _BUFFERS)

The following block is required to create the buffer space the Kernel will
ensure is place at the end of the kernel image proper

	.globl _bufpool
	.area _BUFFERS
_bufpool:
	.ds BUFSIZE * NBUFS

### Data in common memory

_kernel_flag:
	.db 1

## C Level Platform Interface

### Functions that may not be in the discard segment

void platform_idle(void) : This function is invoked whenever there is no
work to run. On a purely interrupt driven machine it can invoke halt
instructions or other power saving. On a polling machine it normally saves
the interrupt state, disables interrupts and does a keyboard poll before
restoring the interrupt state and returning. This causes the serial ports to
be polled at high speed when idle which gives a far better feel.

uint8_t platform_rtc_secs(void) : If CONFIG_RTC was set this routine will be
called by the kernel in order to keep a lock between the system time and
real time. The other fields do not matter.

void platform_interrupt(void) : This function is invoked from the low level
interrupt entry code. It is responsible for handling the results of any
interrupt events and should also ensure timer_interrupt() is invoked ten
times per second. This code can also handle polling devices.

void platform_discard(void) : This is called just before init is executed.
It is able to grow the buffer space (see platform-ubee for an example) or
otherwise reuse memory removed from discard.

bool validdev(uint16_t dev) : Checks whether a device code is valid. This
function needs to live in the same file as the device table and can be
copied as-is from another port.

kputchar(char c): Write a character to the system console. This normally
invokes tty_putc in the terminal interface but can directly map to a debug
port if preferred.

### Functions that may be in the discard segment

void map_init(void) : This can be a null function on a banked system

void pagemap_init(void); This function sets up the page map. For a banked
system this consists of calling pagemap_add() with an 8bit non zero constant
to represent each bank. In many cases this can be the actual value to write
to a bank register. In more complex systems it may be a lookup table index.
This value is the value passed into map_process and map_process_a.

uint8_t platform_param(char *p) : Called with each string on the boot
command line. If a string is recognized as an option return 1 and act upon
it, otherwise 0. This method is invoked after device_init but before we
mount the root file system. It is therefore allowed to modify the device
tables.

void device_init(void) : This is invoked after interrupts are enabled and
allows device probing and setup to occur before we mount the root file
system. It is called after the kernel serial (tty) interfaces are invoked so
the console needs to work before this is called.

### Tables

struct devsw dev_tab[] : This table holds the functions to call for device
driver operations. Each device major number has open, close, read, write and
ioctl (control) methods. Not all of the methods are mandatory and the kernel
provides helper methods - no_open, no_close, no_rdwr, no_ioctl for them.
Also provided is nxio_open which is the open method for a device that is not
present in the system.

See the Fuzix Device Manual for how to fill this in and write device drivers.

