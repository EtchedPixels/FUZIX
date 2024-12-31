# Fuzix for the COCO3

Copyright 2015, Brett M. Gordon, under GPL2.

This port is based heavily on the platform-6809test and the two dragon ports by
Tormod Volden.  It will not, as yet, work on a stock 128k CoCo3; 

## Supported Targets

- Tandy COCO3 SUBTARGET=real
- Tandy COCO3 (emulated) via xroar or mame SUBTARGET=emu
- COCO3 FPGA SUBTARGET=fpga
- COCO3 Nano SUBTARGET=nano

Set SUBTARGET accordingly when building. The default is to build for the
emulator with IDE and a Becker Port

## Supported Devices

- Display (RGB or Composite)
- Keyboard
- Cloud9 style IDE
- CocoSDC
- Drivewire
- SD on FPGA and Nano
- Printer (drivewire only at this point)

## Devices To be Added

- Joystick

## Keyboard

Some work needs done on the support on the ALT key shifting.  The ALT
key is hard to use with a emulators that work in Windows or X, anyway.

The tilde, "~", character can be produced with <F1>
The pipe, "|", character can be produced with <SHIFT><F1>

Pressing <CTRL><1> will switch to virtual console No. 1.
Pressing <CTRL><2> will switch to virtual console No. 2.

## Building

### Tools Required

decb from the toolshed project: http://sourceforge.net/projects/toolshed/
lwtools from William Astle: http://lwtools.projects.l-w.ca/
gcc6809: http://toolshed.sourceforge.net/gcc/

The gcc6809 tools are a patch based on gcc 4.6.  The patch and emulated
ar/as/ld commands can be found at:

http://lwtools.projects.l-w.ca/hg/index.cgi/file/a3711f5ac569/extra

A full set of tools, assembled and ready to go:

https://launchpad.net/~tormodvolden/+archive/ubuntu/m6809

### Compilation

make diskimage will build a complete disk image, including an integrated
boot floppy for hdbdos, as well as an actual boot floppy.

## Booting

### HDBDOS

The raw IDE image will autoboot with the standard HDBLA ROM. If you've moved
the "OS9Offset" then adjust Kernel/platform-coco3/Makefile to match.

### Drivewire

Booting this Fuzix is accomplished via some flavor of Disk Extended
Color Basic (DECB).  When building Fuzix two disk images are
produced. The first, "Kernel/platform-coco3/fuzix.dsk", is a standard
35 track DECB image.  This RSDOS filesystem contains a small
bootloader, "BOOT.BIN", that does the serious work of loading the
actual kernel, "FUZIX.BIN", into memory.  There is a HDBDOS/YA-DOS
style "AUTOEXEC.BAS" file included to assist in auto-booting FUZIX.

The second disk image produced is
"Standalone/filesystem-src/fuzixfs.dsk".  This is the actual Fuzix
"root" filesystem, containing the usual suspects.

The stock build can be booted by placing "fuzix.dsk" in Drivewire as
disk 0, and "fuzixfs.dsk" as disk 1.  PLEASE TURN OFF "HDBDOS
Translation" on the Drivewire Server. If not using HDBDOS/YA-DOS type
the following at the BASIC prompt:

LOADM"BOOT.BIN":EXEC 'DW1

The boot loader will automatically consider anything after the BASIC
REM comment as a commandline to be sent to the kernel.  In the above
example, "DW1", indicates that DriveWire Drive No. 1 should be mounted
as the root filesystem. The boot leader will scan the command line for
"BOOT=filename" to set which kernel image to load.  If no boot image
is specified it will use "FUZIX.BIN".  Regular DECB filename rules
apply, so "BOOT=FUZIX.BIN:1" works as expected.

## Emulation

### Xroar
xroar -machine coco3 -cart ide -cart-rom hdblba13.rom -load-hd0 Images/coco3/disk.img

Options specify -ccr none.

### Mame

Drivewire emulation is the old Coco3 serial line protocol, now
run over a TCP connection.  It lets the Coco3 access disk blocks
by sending requests out its serial port, and having some external
(presumable, bigger) machine access its disks and send back
results.  FUZIX will boot with a DOS boot disk at drive 0, and
the FUZIX filesystem as drive 1.  Both of them are actually
served from the "outside", via Drivewire.

Your goal is to run a Drivewire server on your host machine, with MAME
accessing the two FUZIX disk images you've built.  Your Coco3 runs
on MAME, and when your Coco3 OS (FUZIX!) makes a Drivewire access,
MAME turns it into bytes on a TCP connection to that Drivewire
server.

You will need MAME (Multi Arcade Machine eMulator) of a fairly
modern vintage; Drivewire emulation was added in 0.156 (Debian
Jessie, for instance, only has 0.154).  Check under "Machine
Configuration" to see if there is a "Becker Port" configuration
option, and make sure it's enabled.

A Drivewire enabled Coco3 model has been added to MAME;
if you start MAME as "mame coco3dw1" it will try to start HDB-DOS
with Drivewire support.  The boot ROM files are in coco3dw1.zip,
an archive easily found via a search engine.

Before starting your emulated Coco3,
start up your Drivewire server with Kernel/platform-coco3/fuzix.dsk
as drive 0, and Standalone/filesystem-src/fuzixfs.dsk as drive 1.
With any luck, you'll see a progress bar as FUZIX loads, and then
the FUZIX kernel will start up, running from that second (larger)
dsk image.

There are Drivewire servers out there, some quite powerful.
If you need a quick-n-dirty Drivewire server, one of the FUZIX
enthusiasts has a simple Python based one available:

    https://github.com/vandys/pydw


## Technical Information

Fuzix on the COCO 3 currently runs with 16K logical pages using pairs
of COCO GIME 8K pages. This will change.

The current memory map is

### Kernel mode

| 0000-3FFF | Kernel. Video routines here, Remapped over for swap I/O
| 4000-7FFF | Kernel
| 8000-BFFF | Kernel
| C000-DFFF | Part of user app (will be freed up in move to 8K maps)
| E000-FFFF | Udata and Common (one copy per process)

User Mode

| 0000-DFFF | Application
| E000-FFFF | Common (per process)

The physical map is arranged as follows

| 0-7 | Kernel (in order 0000-FFFF
| 8-B | Screen (mapped over 2000-5FFF). A/B not yet used for anything
| C   | Banked buffers (mostly empty - could move disk buffers here if needed)
| D+  | User space
