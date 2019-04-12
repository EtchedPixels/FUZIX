# Memory Management

Memory management in Fuzix is abstracted from the core kernel. Different
architectures and platforms may implement their own memory manager
functionality. Each process is assumed to consist of the code, data and bss
(zeroed memory) of the process, a gap for dynamic allocation and then the stack. The total size committed can be set via the chmem command.

The memory manager is responsible for providing functions to allocate the
memory for a process, to resize the allocated memory, to free the memory and
to report on the memory available. The finer details of how the memory is
managed are hidden entirely from the core kernel code. 

The kernel also provides a method tmpbuf() which allocates one of the disc
cache buffers to the caller. These are a precious resource and should not be
held across system calls. Driver specific code can use any other platform
specific allocation functions as needed.

Some platforms and memory managers implement additional memory management
and allocators as well as the standard ones.

## Memory Manager Methods

Each memory manager provides the following required functions

int pagemap_alloc(ptptr p)

Allocate memory for the process p.  If no memory is available or the request is
too large to be fulfilled then ENOMEM is returned. Upon success the p->page and
p->page2 values are updated by the memory manager. They can be set however
it wishes in order to indicate the resources allocated.

p->page must not be zero. Zero is used to indicate a process with no memory
currently allocated.

int pagemap_realloc(usize_t code, usize_t size, usize_t stack)

Reallocate the memory for the current process to a new larger size given by
size. Currently this does not need to preserve the existing memory content.
ENOMEM is returned if there is no memory or the request is too large to be
fulfilled. The code and stack values are not currently used but will become
meaningful in future.

int pagemap_free(ptptr p)

Free the memory resources assigned to process p.

usize_t pagemap_mem_used(void)

Reports the number of Kbytes of memory that are currently allocated.

usize_t procmem

Set by the platform the number of Kbytes of memory available for process
allocation. This is used by tools such as 'free' to help report memory
utilization.

usize_t ramsize

The total amount of memory available in the system, including memory used for
the kernel and other resources. This should reflect the way the platform
normal reports total memory. It is used solely to provide feedback to the user
on their system configuration. It should thus match the values reported by
firmware or other convention.

## Eight Bit Platform Defines

The loaders and management for the 8bit systems also expected the following
defines to be present

    PROGBASE: the base address at which programs begin (usually 0)
    PROGLOAD: the base address at which programs are loaded
    PROGTOP: the first byte above the program space

In order to run CP/M emulation PROGBASE needs to be zero, and PROGLOAD
0x100.

## Fixed Bank Memory Manager

This module provides the needed support code and glue to handle systems
where there is a fixed common at the top of memory with banked memory below.
This is probably the most common configuration for traditional Z80 based
MP/M and CP/M systems with memory banking.

The fixed bank manager is one of the easiest to use and has library helpers for
task switching that can support most platforms. It thus makes a good bring up
manager on systems that can actually do more complex banking.

The udata is normally mapped into the end of the bank above PROGTOP and swapped
as one. In most cases this requires the cost of udata copying.

The manager stores the bank number in the process p_page and udata u_page field.
Bank numbers can be any value except zero. It is thus possible in many cases to
simply use the hardware register value needed as the bank number. If there
is a physical bank 0 then this can either be used for the kernel or the
support code needs to convert between the logical and physical numbers using
something simple like an xor or decrement.

The module also provides all of the needed memory manager support for process
swapping.

### Platform Specific Defines

    CONFIG_BANK_FIXED: Select this banking model
    MAX_MAPS: The number of banks that are available to userspace
    MAP_SIZE: The size of the bankable area
    MAP_BASE: The base address of the bankable area

### Fixed Bank Memory Manager Specific Methods

As well as the standard manager methods the fixed bank manager also provides
the following methods:

pagemap_add(uint8_t bank)

Adds a bank to the list of free memory banks available. This is normally
called repeatedly at boot with each of the available banks. There must be at
least two user space banks available.

The fixed bank manager also provides a standard helper library
'z80fixedbank.s' that provides all of the task switching and other
supporting logic required for scheduling and swapping. One assembler routine
is required in the platform specific code

map_process_a: map the bank given by A. It should be in common space and
must not change any registers.

Similar support library routines are available for the 8080 and 8085.


## Flat Memory Manager

The flat memory manager is designed to be used with 32bit systems that have a
single large flat address space in which all processes live. This covers
most platforms using processors such as the Motorola 68000. Memory is
handed out by an allocator into the flat space. Swapping is not supported.
The fork() system call and Posix semantics are maintained by block copying
when needed.

### Platform Specific Defines

    CONFIG_FLAT: Select this banking model

### Flat Memory Manager Specific Methods

void pagemap_switch(ptptr p, int death)

Switch the memory maps around so that the memory of process p appears
mapped at the correct address range. This is normally used internally by the
task switching logic.

uaddr_t pagemap_base(void)

Returns the base address at which the program was loaded. This helper is
used internally by the execve syscall code on 32bit systems.

arg_t _memalloc(void)

arg_t _memfree(void)

These functions are an implementation of the additional user space memory
allocator functions on flat memory model systems. They are directly hooked
into the system call tables.

An underlying kernel memory allocator must also be used. This is normally
the kernel malloc. At minimum the platform needs to have called

void kmemaddblk(void *base, size_t size)

To adds a block of memory size bytes long from the address base to the memory
pool available for kernel allocation. This pool is used both for kernel
allocations via kmalloc and kfree, as well as the process space for
executables in flat memory space. The passed base address must be aligned to
the alignment required by the processor and allocator.

### 68000 Support Code

The 68000flat.S support library code provides all the needed logic for task
switching including interfacing to the flat memory manager pagemap_switch
logic. For other processors you will need to write equivalent code.

## Simple Memory Manager

The simple memory manager handles systems where there is a fast I/O device
but only enough memory to hold one process in RAM at a time. It handles the
required swapping and task switching as well as very very simple process
management. The implementation is designed to be small rather than maximally
efficient. It does not try to share the memory between two small processes,
nor does it include any clever swap I/O optimizations.

Swap is mandatory.

### Platform Specific Defines

    CONFIG_SWAP_ONLY: Select this banking model
    CONFIG_PARENT_FIRST: Run parent first on a fork (for performance), required
    for Z80 support library, recommended for others
    CONFIG_SPLIT_UDATA: Udata should be written separately
    MAXTICKS: This should be set higher (eg 20)

### Simple Memory Manager Specific Functions

The manager itself provides not specific functions for memory handling but
does provide some slightly non standard swap functionality in order to
support the single process model.

For Z80 processors the library file z80simple.s contains all the necessary
glue and support for task switching.

