# Z80-MBC2

## Implementation

This platform uses the standard banked memory model. One lower 32K bank is
assigned to the kernel (Bank 0). The upper memory is fixed, and
bank 1 and 2 are available to applications, one per bank.

During execution the current per process data and stack (udata) live in the
upper bank near the top of memory. When a process is switched out they are saved
into the top of the low 32K space.

## Memory Layout

Kernel
0000-00FF	Interrupt vectors
0100-7FFF	Kernel code

User
0000-00FF	Interrupt vectors
0100-7DFF	User process	(currently 7CFF needs fixing)
7E00-7FFF	Copy of task kernel stack and variables

8000-????	Kernel
????-EFFF	Disk buffers
F000-FDFF	Kernel common area and stacks
FE00-FFFF	Reserved for firmware


## Notes

There are a variety of clever things that could be done to make this platform
more useful but at a cost. Firstly there is a lot of space free in the top 32K
so 8000-AFFF could probably be made user space and copied in and out each task
switch. As a single user machine doesn't switch much the cost isn't too high
and more becomes possible.

Firmware S220718-R120519 or later is required
