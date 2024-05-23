# Fuzix for RCbus with Tadeusz 128K RAM board

This is a banked RAM board that gives us a couple of 32K banks for user
space. Not ideal but usable within limits. User space lies in the upper 32K
of memory as the banking banks the high not low space.

## Supported Hardware

- Onboard CF adapter
- SIO

## TODO
- Add ACIA

## Optional Devices

- CTC card at 0x88 (including SIO B port speed control). Requires channels 2 and 3 are chained.
- DS1302 at 0x0C or 0xC0

## Time Sources

This must be supplied by an external card or the system will hang on
boot. The CTC provides a proper interval timer and is recommended. The RTC
provides a very slow to read clock only.

## Devices To Maybe Add

- TMS9918A video (and vblank timer interrupt)
- PS/2 keyboard and mouse
- External timer on a carrier pin
