## TODO
- Switch to tinydisk
- QUART CTS/RTS
- TMS9918A - stop console flipping when in gfx mode
- Maccasoft video - debug and add video ioctls
- Ioctls on Macca video for modes and access
- Sprite ioctls
- Ioctls to assign consoles to video cards
- Sound
- Debug EF9345 console switch and cursor handling
- EIPC and devgpio
- EasyZ80 and TinyZ80 double check CTC timings and speed (easyz80 should be
  right)
- Maybe bank buffers in upper half of bank 3
- Fix the bank binary generation so that we handle multiple banks in one source
  file to avoid fun with struct arrays of pointers and the like. We probably
  need to do reloc checks on AREA based relocs not just SYM

## Things To Note When Modifying

  * Do not move discard below C300 on a 16K banking setup. There is stuff in
    start.c that requires that C000-C2FF space is free for starting init from
    the kernel.

## Things that don't work

  * Flow control isn't yet enabled for the serial port.

## Stuff To Do

  * Swap (it's there but could do with more debug)

  * Sound support once we have the core sound layer

  * Go the soft IRQ route with fast SIO serial handling for rx interrupts and
    flip buffers. Also raises space issues as we'll need 256 bytes per SIO
    plus the other costs (Can we defer all but console and steal from the
    top of discard)

  * New networking layuer

## Setting It Up

Fuzix on the RC2014 expects a normal PC style compact flash card. Fdisk up the
card leaving the low space free as fdisk tries to do. Place a file system on
it (type 0x7E). Do not at this point place a swap partition on it (0x7F)
although you can certainly reserve on with a different type code.

The loader and attached kernel image needs to be written to blocks 2+.

"make diskimage" will do all the work to generate a file system, CF card image
and emulator image in Images/rc2014/.

if you need to do it by hand

If you are doing this with the emulator then the easiest way is to use makedisk
to create a disk of suitable size and then

	dd if=foo.cf of=foo.raw skip=2
	fdisk foo.raw

	add the file systems etc onto it (either by loopback mounting or
	using dd remembering the start sector given can be used with
		dd bs=512 seek=startsector conv=notrunc ...

	dd if=foo.raw of=foo.cf seek=2 conv=notrunc

When working on a real CF card or the raw file add the bootable image with

	dd if=fuzix.romwbw of=/dev/sdxx seek=2 conv=notrunc

to place the bootable image on sectors 2 and above in the hole the fdisk tool
leaves for a bootable image.

