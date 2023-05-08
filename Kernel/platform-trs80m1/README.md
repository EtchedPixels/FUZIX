# TRS80 Model I and III plus Clones

## Base Systems

- Tandy Model I with Alpha Supermem or compatible and expansion box
- Tandy Model III with Alpha Supermem or compatible
- EACA Video Genie aka Dick Smith System 80 / TRZ-80 / PMC 80/81 with S100 extender and memory card set up to match the supermem (eg a BG256S can be set up this way)
- Tandy Model I or EACA Video Genie  with Selector and expansion box
- LNW80 Model 1 (not graphics modes) with Alpha Supermem or compatible

Some other clones may work providing they can be fitted with a suitable
memory expansion.

In all cases additional banked memory is required either using a Supermem or
compatible card, or the Selector on the model 1. The

### With Other Ports

- EACA Video Genie with EG64.3 memory banking. See "genie-eg64"
- Tandy Model I with Lubomir Soft Banker. See "genie-eg64"
- Tandy Model 4/4D/4P. See "trs80"

### Probably Won't Work

- Genie II/IIs/III/IIIs
- Le Guepard
- Lobo max

## Supported Hardware

### Console

- On board video at 64x16
- Lower case kit (Tandy 26-1104, later Video Genie, or standard mods)

### Graphics

Supported only via graphics interface hooks, not as a console or UDG
graphics

- ChromaTRS (could support a console but not yet done)
- HRG1B
- Lowe Electronics LE18
- Orcim PCG80 : boot option "pcg80"
- Programma Intl. 80 Grafix : boot option "80gfx"
- Tandy HRG (260-9800) : boot option "micro"

### Memory
- Alpha Supermem or compatible
- Selector

### Storage
- Floppy disk
- Hard disk (Tandy 26-1132 or compatible, including FreHD etc)
- Lotech IDE at 0x40 (not with Supermem)
- Percom compatible doubler

### Other
- Alpha Products Joystick
- Tandy Expansion Interface (26-1140/41/42) - required on model 1 for the interval timer
- EACA Expansion Interface (EG3014/X-4010/X-4020) - required on EACA machines for interval timer
- EACA EG3016 Printer
- EACA EG3020 RS232
- EACA EG3022 S100 (no specific card drivers included)
- Holmes style speed up board (anything using port 0xFE bit 0)
- Real time clock at 0xB0, only used for clock locking
- Tandy 26-1145 RS232 (or compatible)

## Unsupported
- Anitek MegaMem (16K window so only useful for ramdisk or swap) - TODO
- FreHD extensions (clock, disk swapping etc)
- LNW80 extra keys
- LNW80 graphics modes
- LNW80 II
- Lowe Electronics FRED (until Fuzix gets audio support)
- M1SE/M1RE (except for compatible features)
- Tandy style double density kit (26-1143)
- TRS80 style serial printer on Video Genie
- TRSnet

## Installation

make diskimage
Set up hard1-0 as a hard disk image on a FreHD
Put the relevant boot.jv3 disk on the FreHD and use the FredHD apps to make
a floppy disk of it.

## Emulator Notes

The xtrs emulator appears to have a problem with serial handling defaults.
You may need to specify -serial "" on the command line to avoid the model 3
hanging at the boot prompt.

Always specify "-emtsafe"

Copy boot.jv3 to disk1-0 and use hard1-0 from the emulator with xtrs or a
FredHD. Copy boot.jv3 to disk3-0 and hard1-0 to hard3-0 for a model 3
emulation.

You will still need a boot floppy but you can make those using a FreHD or
other tools from the JV3
