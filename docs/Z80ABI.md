# 8080 and Z80 ABI

Note: This describes the ABI that will be implemented not the current state
of affairs. This is a work in progress as the syscall ABI migration occurs

## Sizes

All pointer types are two bytes. All integer types are two bytes. All
character types are one byte. Long values are four bytes. There is no
floating point ABI to the kernel. The way a char is pushed depends upon the
8080 or Z80 compiler but this is never exposed as a kernel ABI. Long values
are always passed through the syscall interface as pointers to memory.

The time_t values passed from the kernel are 64bits and passed by copying
them to memory not as a return value. The internal behaviour of the user
space time_t is compiler dependent, but note that the kernel time is
unsigned relative to midnight GMT January 1st 1970.

## System Call ABI

System calls are made via the jump placed in the code overlaid over the
program header at load time. Offset 0 from the load address is the required
code to invoke a system call.

[Not yet implemented: currently using RST 30/RST 6 but this has to change
 for systems with fixed ROM in the low space]

The Z80 system call ABI will change the contents of AF, DE, HL and AF'.
Other registers are not changed. BC is preserved as it is the 8080 frame
pointer.

The system call number should be in the A register on entry. The arguments
should be in C order on the stack and the kernel then expects two levels of
return address below them at the point of entry.

On return the carry flag is set on an error. If there is no error then HL
holds the return of the system call. If there is an error then HL holds the
errno code.

This allows the kernel syscall interface to be as follows

````
	_syscall_n:
		ld a,#n
		jp _syscall

	_syscall:
		call _entrypoint
		ret nc
		ld (_errno),hl
		ld hl,#-1
		ret
````


and on the 8080/8085

````
	_syscall_n
		mvi a,n
		jmp _syscall

	_syscall:
		call _entrypoint
		xchg
		rnc
		xchg
		shld _errno
		lxi d,-1
		ret
````

The 8080 compiler uses DE for the C return value. The Z80 compiler uses HL.
The kernel always returns the values in HL.

## Program Entry

The program is loaded at a page aligned address. The code is placed first,
then any data and finally cleared BSS space. On entry the DE register holds
the load address of the program. SP points to the top of the free stack
space. Above it are the arguments and environment. The stack has a pointer
to the argv argument list and the number of arguments pushed on it before
entry. Above this is the environment.

Programs are expected to do their own relocation.

## Binary Header

To do when the new format is implemented


## Signals

To do
