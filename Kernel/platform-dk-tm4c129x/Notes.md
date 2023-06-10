## SD Card

The dk- SD card is on

PQ0: SSI3 clock
PQ2: SSI3 data out
PF0: SSI data in
PH4: SD card \CS

The ek- card has no onboard SD and PH4 is not available

PQ0: Breadboard 51
PQ2: Breadboard 55
PF0: Breadboard 66
PH4: needs alternative ?   57 = PQ3 ?

## OpenOCD

To download the image

openocd -f openocd.cfg &
arm-none-eabi-gdb -x load.gdb fuzix.elf

The console will be forwarded via the debug USB as an ACM serial device at
115200. So something like

gtkterm -p /dev/ttyACM0 -s 115200

will do the trick.

