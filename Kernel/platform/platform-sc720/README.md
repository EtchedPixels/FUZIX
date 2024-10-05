# Fuzix for the Small Computer Central SC720

This is a less suited platform than the conventional 512/512K builds as it
lacks the paged MMU and has no timer. In particular the user program size is
limited to just under 32K per program and thus disk sizes to 8MB in order for
fsck to be able to operate in the avaiable process space.

## Supported Hardware

- SC720 SBC
- Onboard CF adapter
- Onboard SIO

## Optional Devices

- CTC card at 0x88 (including SIO B port speed control). Requires channels 2 and 3 are chained.
- DS1302 at 0x0C or 0xC0

## Time Sources

The SC720 lacks either a period timer interrupt or a real time clock. One of
the two must be supplied by an external card or the system will hang on
boot. The CTC provides a proper interval timer and is recommended. The RTC
provides a very slow to read clock only.

## Devices To Add

- TMS9918A video (and vblank timer interrupt)
- PS/2 keyboard and mouse
- External timer on a carrier pin

