# 68HC11 ABI

## Memory Layout

The program image is loaded at the base of the user memory, with the
bss following. The brk and sbrk system calls allow the adjustment of the top
of available memory. The stack extends down from the top of the user memory.

A range of direct page addresses is allocated at load time. The memory bank
switching normally means each binary will have its own private direct page.
This is however shared with the kernel (64 bytes) and some space may be used
for the I/O devices depending upon the internal I/O location on the
platform.
 
## Sizes

All pointer types are two bytes. All integer types are two bytes. All
character types are one byte. Long values are four bytes. There is no
floating point ABI to the kernel. Long values are always passed as pointers
through the system call interface

The time_t values passed from the kernel are 64bits and passed by copying
them to memory not as a return value. The user space time_t is 64bit. Note
that the kernel time is unsigned relative to midnight GMT January 1st 1970.

## System Call ABI

System calls are made via the jump placed in the code overlaid over the
program header at load time. Offset 0 from the load address is the required
code to invoke a system call and is invoked with JSR.

On entry the A register should hold the call number. The B register 0x80.
The arguments at the point before the JSR should be arranged with the
first argument top of stack, then two bytes are skipped, then the remaining
arguments. Arguments are stacked in normal C order so the first argument is
lowest on the stack. The two skipped bytes allows for the fact that the C
ABI passes the first argument in a register. The syscall library routines
push this thus there is a return address slot in the arguments.

On return the D register holds the result and the X register holds an error
if present or zero if not.

## Program Entry

All 68HC11 binaries are required to be self relocatable. They are entered
with
	A = load address high byte (always 256 byte aligned)
	B = direct page base
	X and Y are undefined

Example self relocation code can be found in crt0nostdio_68hc11_rel.S in the
Library/libs tree.

On entry the stack holds a pointer to the argument vector, then the argument
count and this is followed by the environment, and after that the argument
data.

## Binary Header

The standard binary header is used. There are no platform specific hints
defined. The a_zp field should be set to the number of direct page bytes
required.

## Signals

Because the C compiler does not generate re-entrant code all signal handlers
are despatched to the routine whose address is stored at address 16 (just
after the header). This routine is invoked with

	D = signal number
	X is undefined
	Y = handler address

When the handler code is invoked the top of the stack is a return address
to unwind from the signal handler, and above it is anothr copy of the
handler address (this is needed for non 68HC11 specific code that cannot use
the Y register).

## Compatibility

The 68HC11 kernel can (or will be able to) run 6800, 6803 and 68HC11
binaries. It cannot run 6303 binaries as the 6303 instruction set is not a
68HC11 subset. Non 68HC11 binaries use the 6800 system call interface.
68HC11 binaries must not use the 6800 system call interface.
