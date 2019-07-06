## A FUZIX target for the JeeLabs Retro Z80 emulator

> Note: this port was adapted from the `z80pack` and `sbcv2` ports

JeeRetro is configered with a simple 4K fixed common and 60K fixed sized banks
(this can be changed at runtine, see the "`common`" emulator request below).

We run FUZIX with one application per bank and the memory map currently is:

### Bank 0:

``` text
0000-0080	Vectors
0081-0084	Saved CP/M command info
0100		FUZIX kernel start
????		FUZIX kernel end ~= A000
(big kernels go up to E400 or so!)
no discard area, current kernel easily fits in 60K
End of kernel:	Common >= 0xF000
                uarea
                uarea stack
                interrupt stack
                bank switching code
                user memory access copy
                interrupt vectors and glue
                [Possibly future move the buffers up here to allow for more
                 disk and inode buffer ?]
FFFF		Hard end of kernel room
```

### Bank 1 to Bank n:

``` text
0000		Vector copy
0080		free
0100		Application
ECFF		Application end
ED00-EFFF	uarea stash
```

Disk swap device is needed due to limited number of user banks.

Disk layout depends on the emulator configuration
One option is to put the kernel at block 1, and the fs starting at block 2048
This is compatible with a possible future use of MBR partitioning. I.e.

    dd if=fuzix.bin of=drivea.cpm bs=512 seek=1 conv=notrunc
    dd if=rootfs.img of=drivea.cpm bs=512 seek=2048 conv=notrunc

See <https://jeelabs.org/2018/z80-zexall-f407/> for further Jee Retro details.

### Emulator details

The emulator used for this port is based on "`z80emu`", which can be found on
[GitHub](https://github.com/anotherlin/z80emu).

As with the `zextest.c` demo in that project, there is a somewhat odd way of
calling into the emulator for I/O, etc:

* each `IN` instruction is treated as a system call
* any `OUT` instruction will end the emulation
* the `HALT` instruction does nothing at the moment

The oddness comes from the fact that "in" need not mean input, and that any of
the registers can be used to pass information in or out.

The latest version of z80emu used for this port can be found at
[git.jeelabs.org](https://git.jeelabs.org/jcw/retro/src/branch/master/z80emu).

### Emulator System calls

Note: these calls and the way information gets passed around are still in flux.

> One idea would be to switch to a "`HALT + .db <NN>`" sequence for emulator
> requests, and leave IN & OUT requests for other purposes (perhaps general
> file-/stream-based I/O).

| IN | Description | Notes |
|---|---|---|
| 0 | readable | returns A=0xFF if a console char is pending |
| 1 | getc | waits and returns the next input from the console in A |
| 2 | putc | write C to the console output |
| 3 | puts | write zero-terminated string in HL to the console |
| 4 | disk I/O | multiple registers, see below |
| 5 | rtc | get/set the time, C=0/1 resp, 5-byte buffer in HL |
| 6 | common | set common base, A=common/256, returns #banks in A |
| 7 | bank | select bank A, previous value returned in A |
| 8 | xmove | cross-bank copying (not used yet) |
| 9 | regs | print all main register values as one line to the console |

Disk I/O register use:

* A = drive (only 0 = root, 1 = swap, for now)
* B = sector count (0..127), +128 if writing (else it's a read)
* D = track number, E = sector number, both 0-based
* HL = buffer address
* status is returned in A (not yet, currently always zero)

Sectors are 512 bytes in the FUZIX version, and 128 bytes in the CP/M builds.
