# TODO

- devinput support (joystick, keyboard)
- switch to 8K banking
- 640x200 mono graphics support ?
DONE - 32K screen space (reclaim 3f, move user up, what is 10 used for ?)
- Mouse ??? https://nickmarentes.com/ProjectArchive/hires.html
YES - Can we then fit in networking and coco ethernet stuff or COCOIO (W5100)
  direct connect at 0xFF68-FF6b (ctrl, addr hi, addr lo, data)
- 2MB RAM support
- FIXME: double check graphics ops versus low map
- 32K video, with helper ops doing bank flip on the right line
  Will need some thought for best results - do we want to put
  one console in each 16K block and except for 32K modes allow
  them to co-exist and switch ?

# Brett's Old Notes (mostly obsolete)

# Fuzix for CoCo3 (512k)

Copyright 2015, Brett M. Gordon, under GPL2.

This port is for running FUZIX on an emulated 512K Ram Color
Computer 3.  This port is based heavily on the platform-6809test and
the two dragon ports by Tormod Volden.  It will not, as yet, work on a
stock 128k CoCo3.


*************************
REQUIREMENTS
*************************

A working DriveWire implementation.
A 512k CoCo3 (emulator only for now)

and for building:

decb from the toolshed project: http://sourceforge.net/projects/toolshed/
lwtools from William Astle: http://lwtools.projects.l-w.ca/
gcc6809: http://toolshed.sourceforge.net/gcc/

The gcc6809 tools are a patch based on gcc 4.6.  The patch and emulated
ar/as/ld commands can be found at:

http://lwtools.projects.l-w.ca/hg/index.cgi/file/a3711f5ac569/extra

A full set of tools, assembled and ready to go:

https://launchpad.net/~tormodvolden/+archive/ubuntu/m6809


*************************
BOOTING
*************************

HDBDOS
------
make diskimage

The result will be Images/coco3/disk.img which is a raw IDE disk image
with the necessary boot floppy integrated into it as a virtual disk.

For emulation purposes this can be booted with

xroar -machine coco3 -cart ide -cart-rom hdblba13.rom -load-hd0 Images/coco3/disk.img

You probably also want to specify -ccr none

Drivewire
---------
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


*************************
KEYBOARD
*************************

Some work needs done on the support on the ALT key shifting.  The ALT
key is hard to use with a emulators that work in Windows or X, anyway.

The tilde, "~", character can be produced with <F1>
The pipe, "|", character can be produced with <SHIFT><F1>

Pressing <CTRL><1> will switch to virtual console No. 1.
Pressing <CTRL><2> will switch to virtual console No. 2.


*************************
DEVICES (so far)
*************************

node       major   minor     description
/dev/tty1  2       1   	     console / virtual terminal No. 1.
/dev/tty2  2       2         virtual terminal No. 2.
/dev/tty3  2       3         Drivewire Virtual Window #0
/dev/dw?   8       0-256     Drivewire Block Drives
/dev/lpr   3       0         Drivewire Printer

**************************
BUILDING
**************************

# 1. Build the kernel:
make -C Kernel TARGET=coco3

# 2. Build the boot disk:
make -C Kernel/platform-coco3 fuzix.dsk

# 2. Build the libraries
make -C Library tools/syscall_6809
make -C Library/libs -f Makefile.6809 TARGET=coco3

# 3. Build the utils
make -C Applications/util -f Makefile.6809 TARGET=coco3
make -C Applications/V7/cmd/sh -f Makefile.6809 TARGET=coco3

# 4. Build disk tools
make -C Standalone

# 5. Build boot disk image
cd Standalone/filesystem-src
./build-filesystem -X fuzix.dsk 256 65535

(Note that Kernel/platform-coco3/build is a script which
basically does all this, including pre-cleaning previous
builds output.)

*************************
NOTES ON USING MAME
*************************

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

*************************
DONE
*************************

* Fix the underlying Banking layout to better handle UDATA


*************************
TO DO
*************************

* Finish SDC Drivers Need ioctl for meta methods, userspace cntl program.

* SCSI Drivers.

* Better support of the GIME chip's video modes

* A whole gaggle of things, to numerous to count.


*************************
BUGS
*************************

* Things work better if you compile the userspace utilities (esp. "init")
with standard gcc optimizations.


