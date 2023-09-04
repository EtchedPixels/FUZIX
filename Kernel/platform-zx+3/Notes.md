# ZX Spectrum +2A/+3


## Mmeory Model

The memory mapping on this machine is a bit friendlier but it's still single
(large) process in memory at a time to get the best result. Banked kernel
2 x 32K is also doable but probably not such a good use of the machine

We use +2A/+3 'special paging mode'. It's special, but limited. Sufficient for
a nice one process in memory model.

One quirk of this box is that banks 4-7 run at an effective 2.5-3Mhz due to
video wait states but 0-3 run the full 3.5. Thus we want to land some things
in 0-3 and should ponder that carefully.

The plus 3 memory map is fairly simple.

There are four maps

0/1/2/3
4/5/6/7
4/5/6/3
4/7/6/3

and the video can be at the bottom of 5 or 7

As we can't get 5 anywhere convenient video has to go in 7

We run with the following mappings

| User	| 0 / 1 / 2  | Common 3
| Kernel  | 4 / 5 / 6  | Common 3
| Video   | 4 / 7 / 6  | Common 3

Bank 7 is only partially used at this point

| 0000-17FF | 	The 6K of screen bitmap
| 1800-1AFF |	Attributes
| 3D00-3FFF |	Font

So we could possible use external buffers but it isn't clear what gain
there would be if any.

Annoyingly there is no easy way to do direct video except maybe once we
do the video in loader hint hack then we can flip the screen to 5 and
load an app from 5D00-wherever. Still ugly.

## TODO

Fix memory size reporting 64 v 48K

Floppy drive B - set drive info and propogate it

Floppy drive B - set parameters at init time

Q for all these machines - if ports in 4000-7FFF range are contended does
it speed up our divide/divmmc code to unroll it and also set b to 0xFF
every 128 so that we never contend

Eventually consider trying to do thunked single process with C000 sized
user but screen mapped and buffers in rest of 7 ?

0-3		Kernel
4-7		User, video in 7

May also be able to later do

0-2		Kernel		3 high kernel common
				7 upper 8K buffers and high stubs
				7 lower screen
4/5/6		User 64K	3 high kernel common , 3 low user

4/5/6		User 48K	7 screen, upper 8K buffers


0/4 hold page0 copies, 3 7 both hold pageh copies - fine as video is
in bottom of that chunk.

Would give direct mapped video

Turn on interrupts during swapping

Debug generic 8bit IDE support on emulator
