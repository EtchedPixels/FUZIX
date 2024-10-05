# FUZIX for Mini-M68k board

## Installation

1.  Upgrade the BIOS to version 10.3.  This is mandatory.
4.  Boot the Mini-M68k board with the MF/PIC used for the serial console and the CF card attachment.
5.  At the command prompt, boot Fuzix with either BOOT.CMD or FUZ.CMD.  Note that Fuzix must be booted in supervisor mode.  The command files insure this.
6.  Log into Fuzix as ‘root’; there is no password.
7.  Terminate fuzix with ‘bye’ or ‘shutdown’.

## Implementation Notes

1.  The BIOS must be upgraded to 10.3 or later.  Either BOOT.CMD or FUZ.CMD may be used to boot FUZIX from the BIOS shell.  
2.  The FUZIX filesystem is specified at the prompt “bootdev:” as ‘hda2’ (no quotes).
3.  Log in as ‘root’.  There is no password unless you later create one.
4.  The system you have booted is closely akin to Unix System V, version 2.  Some of the source code of the utilities comes directly from the heritage Unix V7 release.
5.  Always shut the system down with ‘shutdown’.  Do not use ‘halt’ or ‘reboot’ as only ‘shutdown’ properly dismounts the file system.  If you goof, you will have a lengthy file system check at the next boot (or two).  Do the full check.
6.  The original Bourne shell from V7 Unix is not as sophisticated as Linux ‘bash’, the “Bourne again shell.” 
7.  My opinion is that ‘ls -l’ is better than separate utility ‘ll’.  ‘sh’ does not have aliases.

## Memory Map

Memory Map on the RetroBrew/N8VEM Mini-M68k SBC (BIOS 10.3)

````
Start	End	Length
000000	0000ff	000100		64 interrupt/trap vectors
000100	000b97	000a98		BIOS .bss   No 68000 vectors here!!!
000b98	000fff			*unused*

001000	00ffff	00e000		FUZIX .bss  or loaded code
````

Thus if FUZIX .bss is < 0xe000, then 64K memory protection may be enabled on
the Mini-M68k board, version 2.  Version 1 has no memory protection.

BIOS code starts at 00FF80000	512K - 64K
 I/O area starts at 00FFF0000	64K

The NS32202 Interrupt Controller registers are addressed in the area:
	MF/PIC board address (0x40) + 0rrbb
	rr = internal 202 register
	bb = MF/PIC board address bb==40
Since rr uses only 6 of 8 bits, all I/O addressing may use addresses
from 0xFFFF8000 to 0xFFFFFFFF; for debugging, switches and lights are at
address 0xFF, easily addressed as 0xFFFFFFFF == (-1).

10/10/2022 JRC

## TODO
- [DONE]Rework the serial driver to be modular to plug in devices
- [PART]Sort out RTS/CTS in the 16x50 driver (and core to some extent)
- [DONE]Add tty_inproc_bad for level 2 systems to honour PARMRK etc
- [DONE]Add the RTC back in and remove the date trickery (start.c etc)
- Switch to 64 vectors ?
- Check the PPIDE code versus Will's latest optimizations
- [DONEish]DualSD support ? What other cards are relevant
	- Need to update core code to support not present bit capability
- Pick up BIOS baudrate
