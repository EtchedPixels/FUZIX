# Fuzix for RCBUS Z180 systems booting from RomWBW

Based upon the N8VEM Mark IV port by Will Sowerbutts <will@sowerbutts.com>

## Cards
- Z180 CPU card (SC111, SC126, SC130, SC131, Z80.no #25d, #70a etc)
- 512/512K memory card with RAM in the upper 512K (SC119 or similar)

  18.432MHz clock is assumed (see kernel.def and devtty.c timing table)

## Options
- RC2014 CF adapter at 0x10
- SC126FIX CS expansion
- Wiznet 5500 on SPI

## Not yet supported but planned:
- RC2014 serial/ctc interfaces (only the onboard ports are handled)
- RC2014 TMS9918A

## Not supported
- RIZ180 (128K/128K RAM, different boot monitor)
- SC111 with SCMonitor firmware

## TODO
- Test "turbo" option for clock doubling
- Add wait state control option for mem/io/cf wait states
- More CF testing
- Can we do CF with DMA1 ? (ought to get us to 3MBytes/second which is about the PIO 0 limit with 0 mem 1 ide wait). 36MHz would get us closer to 6 but less due to wait stating so probably ok as PIO1.

## Platform Level Differences

### RCBUS Z180

SC111 and similar cards having a Z180 and ASCI based serial but
without other features. We probe for an RTC at 0x0C but this is
optional. IDE CF at 0x10 required.

### SC126

Enables the SPI SD card interface and WizNET 5500
If you are using both an SD card and WizNET at once you must
have a recent BIOS snapshot (not 3.0) or an SC126 SPI expander
fitted. If your SD card doesn't properly leave the bus when not
selected you'll need an SPI expander anyway. The LEDs are driven
as blinkenlights.

There is a mod for 1MB RAM which is also supported.

### SC130

Only a single SPI CS is available. SD card is supported but you'll
currently need to tweak the code to use W5500 instead as it assumes
port 2 and the ROMWBW image doesn't report the SC126/30/31 differently.
If you set it to use port 1 it should work fine.

Adding SPI support for other boards involves identifying how you will control
the CS signal and teaching the SPI code.

## Limitations

Memory is very tight because of the way the Z180 banking works. There is
room for a lot of drivers or networking but not both. This should improve
in future because the network code is designed to be bankable and the
basics are now present for loadable kernel modules.
