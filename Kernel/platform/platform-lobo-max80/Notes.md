# MAX LOBO-80

## Requirements: 

128K RAM
SASI hard disk drive

## Memory Map:

Kernel

0000-0FFF	I/O space and holes (holes not yet used)
1000-FFFF	Kernel (common space low)

User (bank B 1 and B 2)

8000-FDFF	User
FE00-FFFF	udata stash

TODO
- Debug SASI
- RTC delay timings
- Test RTC
- Test Video
- Hardware scrolling
- FDC driver
- UDG graphics
- Graphics ioctls
- Serial port test/debug
- UVC ?
- Use the holes for stuff
- 64x16 mode config
