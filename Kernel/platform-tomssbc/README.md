# Fuzix for Tom's SBC version C with mods.
## Versions

For an EPROM based build that requires no board modifications see
the tomssbc-rom target.

## Hardware

This is a port to Tom's SBC. There are two hardware modifications required.
The first is to bend up A16 on the RAM so it is not grounded, and instead
wire it to pin 3 on jumper P7 (ROM_PAGE15). You can leave the jumper on 1-2
for Vpp on the 32K 27C256 usually fitted.

This modification makes the ROM_PAGE15 pin control the switching of the RAM
instead of allowing for a 64K ROM with four 16K banks (nothing AFAIK uses
more than a 32K ROM).

The second modification is to bend up DCDA on the SIO-2 and wire it to a
8Hz square wave source. The board as designed has no timer source as CP/M
2.2 doesn't actually need one. There are plenty of ways to make an 8Hz
square wave circuit, including these days just using one of the tiny PIC
devices to count it.

(Other frequencies will do but I happen to own an 8Hz source). Set the
clock frequency in config.h and fiddle as needed with devtty.c as the
kernel needs to see a clock divisible by 10 so some fudging is done).

Finally you need to inject the ROM with a helper function in the space at
the top as the monitor for the system doesn't have this included and it
is needed to bootstrap software using both RAM banks. See README.ROMPatch.

## Memory map

````
Kernel

0000-00FF	Vectors (present in both banks)
0100-EFFF	Kernel (common above 0x8000 for ROM dodging)
F000-F1FF	UDATA
F200-F2FF	Istack
F300-FFFF	Common (mostly valid in both banks)

User

0000-00FF	Vectors (present in both banks)
0100-EFFF	User space
F000-F1FF	Kernel stack alternative (used during some bank switch ops)
F200-F2FF	Istack alternate (we may take interrupts in either bank)
F300-FFFF	Common (mostly valid in both banks)
```

## Things To Do
- Build a suitably high CP/M emuation and test it
- Faster interbank copying
- Swap only needed blocks for speed up

## Emulation

````
make diskimage
searle -r scm.rom -T -i emu-ide.img -B -t
````

#Real Hardware

````
make diskimage
````

Boot the machine and in the monitor run the CPM command.
