# Fuzix for the Thomson TO8 and TO9+

## Supported Hardware
-	Thomson TO8/TO9+/TO9D (512K needed currently)
-	Keyboard
-	Video (including graphics mapping and mode switches)
-	SDDrive
-	Floppy Disk (via Monitor)
-	Light pen (via Monitor)
-	Mouse (via Monitor)

## Unsupported
-	Original TO9 (see TO9 port)

## Devices To Be Added
-	SDMoto
-	SDMO
- 	Joystick
-	Sound

## Things Pending
-	Switch to 80 col at boot so monitor reports mouse at 640 pixel
	(1B 5B)
-	Palette setting
-	VTINK/VTPAPER, support for text in other modes

## Setting It Up

Build with "make diskimage". This will produce a disk.img which is a raw SD
card image, and boot.fd which is a floppy image. It shouldn't be hard to make
a cartridge boot, and the TO9 port does this already but not this one.

Stick the floppy in the machine, select shift-2 to boot BASIC 1 which will then
load the bootblock and boot Fuzix.

## Emulation

Not currently. DCMOTO appears to have no option to take a true raw disk image
but nails some fake sectors on the front. Teo doesn't support the sd devices.

