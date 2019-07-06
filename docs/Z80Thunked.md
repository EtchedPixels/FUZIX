# Z80 Special Cases

## Z80 Systems With a 32K Split

There are two classes of system that fit this category. Those with a 32K
fixed bank and those with two 32K pages. For the fixed bank case see the
description of banked Z80 systems. Almost all base Fuzix apps will run in 32K.

If you have two switchable 32K banks then the banking requires some special
handling. The conventional Fuzix model is that you have a common space which
holds writable stacks that are used as you transition between modes. When
you are executing with a pair of 32K banks however you don't have enough
room to make that model work nicely.

There are two ways a 32K split can be handled. Firstly it is possible to
treat it as a single bank model. In this case the high (or low) 32K stays
pinned and the other 32K chunks become one kernel space and the rest are
process spaces. For a 128K system this is probably the better model unless
it has very fast swap. The TRS80 model 4 uses this model in part for the
lack of memory reason and also because the banking rules are quite awkward.

The second approach is to actually do 32K/32K banking. In this model there
are some signifcant changes to behaviour.

Memory is laid out as

	32K low bank:
		Kernel code (not data or bss so we can make kernel/user
		copies easier). Not stacks
		User space + stubs in low 256 bytes

	32K high bank:
		Kernel code/data/stacks/udata
		User space + udata stash

## Entry and Exit

System calls are wrapped with an entry function that switches the
upper 32K to the kernel upper space and on return switches back to the user
32K matching the 32K upper at the time of return. Remember you might be
swapped out while asleep in the kernel! The page number is returned in A
to help. The stub is responsible for stack switching and must save the old
sp in U_DATA__U_SYSCALL_SP.

Interrupt entry is expected to do full register saves, map the
upper 32K and enter the kernel platform code. On return it needs to set the
lower 32K to match the upper 32K.  The page number is returned in A to help.
The stub is responsible for stack switching and saving the old stack
in istack_switched_sp.

Remember you can also take an interrupt in kernel mode in which case the map
is already present and you need to return to the kernel (A will be 0)

The platform syscall wrapper is responsible for saving IX and loading BC DE HL IX
with the syscall arguments 1-4 and A with the syscall number. This is needed
as the user stack may be inaccessible once the entry function switches the
high bank. The wrapper needs to be in the low 256 bytes.

The return from a system call is as follows

	A = page to map high
	DE = retval
	H = signal (or 0)
	L = errno
	BC = signal vector

The wrapper must implement the following logic

	map page A high
	if H then
		push hl
		push de
		push signal number
		push a return helper
		push bc
		ret
	helper:
		pop de	; discard signal
		pop de
		pop hl
		ld h,0
	endif
	ld bc,0
	pop ix	; we saved it on entry remember
	ret

Execve is also via a helper. The caller must provide a low memory function.
Note that the function is called with the user sp set, so may have no valid
stack on execution until the high page map completes.

    _platform_doexec:
	map high user page (passed in A)
	ei
	jp (hl)


The return from an interrupt is as follows (so the handler must save
HL/DE/AF). The wrapped code will save IX/IY/alt-regs/BC.

	HL = signal vector
	A = page
	E = signal

The wrapper must implement the following logic

	map page A high (0 = kernel)
	switch stacks back
	if E then
		push signal number
		push return helper
		jp (hl)		
	helper:
		pop hl	; discard signal
	endif
	pop hl		; assuming the wrapper pushed them as af/de/hl
	pop de
	pop af
	ei
	ret		; not reti



## Function Changes

The standard lowlevel-z80 and z80 usermem functions are not used.
The alternative lowlevel-z80-thunked handlers are used instead.

There is no conventional map_kernel, map_process, map_process_always
or map_process_a. Instead the routines provided are

	map_kernel_low	-	maps the low kernel pages. Stack must be
				high.

	map_user_low	-	map the current user page back into low
				memory

	map_page_low	-	map a given page in the low 32K for user
				access. Stack must be high.

	map_restore_low	-	restore the map from map_save_low

	map_save_low	-	save the old low mapping and switch to
				kernel

This is necessary because high page switches have to be inlined as there is
no easy watch to do stack switches.

The user copy methods must be placed in the upper 32K. They map the needed
bank into the low 32K and copy taking care of any wraps to flip the low
32K around as needed.

Device drivers cannot simply map user space because there is no common stack
space. Instead they too must handle mapping the 32K bank, and the corner
case of a transfer splitting a block.

To help with this the following are provided

	map_user_low	HL = user address BC = length
			DE = kaddr (optional)
			IX = page ptr

	returns Z and HL = address to write/read, BC = length
	returns NZ for 'would split', 
		HL = address, BC = length before split, DE = kaddr
		HL' = user address of second, BC' = length of second,
		DE' = kaddr of second
		(The ' values are intended to be fed to a second call)


Drivers can then decide whether to handle the split or to double buffer the
odd time it happens.

For example

	; HL = user address BC = length
	; DE = kernel destination

	call map_user_low	; Map what we can
	jr z, copy_in_one	; Single map
	ldir
	exx
	call map_user_low	; second map cannot split length doesn't matter
    copy_in_one:
	ldir
	jp map_kernel_low

With inir/otir it's a shade more complex


    sectors_in:
	ld b,#2
    sector_loop:
	push hl
	; 256bytes from disk to user
	; HL = user
	; BC = length
	ld bc,#256
	call map_user_low	; Map what we can DE data is junk...
	jr z, copy_in_one	; Single map
	ld b,c
	ld c,port
	inir
	exx
	call map_user_low	; second map cannot split length doesn't matter
    copy_in_one:
	ld b,c
	ld c,port
	inir
	pop hl
	inc h
	djnz sector_loop
	jp map_kernel_low

Swapping is expected to use the functionality above but with
the map_swap helpers. As swap is aligned in 512 byte chunks from
0 the splitting case goes away and this looks like other multi-bank
swap arrangements.

The MMU hooks are not supported in this model.


It's possible to use this model with other banking arrangements, eg with a
16K banked system in order to maximise kernel memory available. However
because the whole space is flipped to a kernel space this adds the same
overhead as a conventional single bank arrangement in order to copy the
udata.


## Systems with 64K switching via ROM helper

These systems use the same basic interfaces except that

- The helper must switch the whole address space

- map_page_low and map_kernel_low don't do anything but remember the
  selected page

- The rom must provide a routine to copy the initial common code from the
  starting bank to other banks

- The entry code as with 32K ends up doing something like

	ld (switch_sp),sp
	di				; if syscall
	ld sp, kstack			; one for syscall one for irq
	ld a,kernel
	out (x),a			; kernel map
	ei				; if syscall
	; Switches to the same code in the other bank and the kstack
	call helper
	di				; if syscall
	out (x),a			; user page in A
	; back on other stack - this means we can't push/pop over these
	; boundaries so we may need to save alt regs or ix/iy and use them
	; for the stubs
	; A BC DE HL tell us what to do

	...

- Use the 64K via ROM functions.


The final case is systems without a common memory where you can write
through pages or where the shared memory has awkward side effects (eg a
Nascom). This is basically the above example but with RAM based helpers written
through into the top of each bank which implement the functions specified.
Take care that you use a private all bank stack for them or that they
block interrupts.

For all these cases task switching is done single bank style. The kernel
helpers in tricks.s save/restore a common udata. The actual switch may cause
temporary banks switches to shuffle udata but does not cause any visible
ones.

