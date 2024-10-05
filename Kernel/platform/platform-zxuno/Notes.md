# Fuzix for the ZX-Uno / ZX Spectrum SE

## Memory Model

Our memory model is based on the base memory with the Timex MMU enabled. Alas
due to bugs in the ZXUNO we have to use the DIVMMC space for the low 16K which
restricts process size and generally messes things up.

Kernel
0000-3FFF	DIVMMC 0 and 3 (low 8K RO) Common
4000-7FFF	Bank 5
8000-BFFF	Bank 2 (8 on SE)
C000-FFFF	Bank 3 (7 when doing video)

User

0000-3FFF	DIVMMC 0 and 3 (low 8K RO) Common
4000-FFFF	From dock or ex-rom (RAM) space

Graphics apps could end at BFFF and map video C000-FFFF in theory

## Internals

The ZX Uno memory prioritization works like this as far as I can tell

Low 16K		DIVMMC, Dock/EXROM, +3 mode, ROM
16-32K		Dock/ExROM, +3, page 5
32-48K		Dock/ExROM, +3, page 2
48K-64K		Dock/ExROM, spectrum MMU, +3, page 0

Note: Zesarux appears to disable +3 paging when timex mode is enabled but the
Uno does not seem to.

## Spare Memory

### Spectrum SE

- DIVMMC page 1 and 2 (16K) (maybe more if DIVMMC 128K used)
- Dock low 16K
- EXROM low 16K
- Banks 0,1,2,4,6 (80K)

Total of 128K. (2 swap slots) (224 if 128K DIVMMC)

### ZX Uno
- DIVMMC page 1 and 2, 4-15 (112K)
- Dock low 16K
- EXROM low 16K
- Banks 0,1,4,6 (64K)

Total of 208K (4 swap slots)

To use this we need to add some kind of swap frontend driver that replaces
the first used swap pages (the most used) with the RAM

## TODO
- look at using DMA engine
- 80 column
- Graphics apps loaded below C000 and with video map ?
- Sound, including DMA sound ?
- Font and video ioctls

## DMA

Sees CPU mapping

Write register to FC3B
Write data to FD3B low/high if 16bit

r A0 - ctrl	
	bits 1-0: mode
	00 stopped
	01 burst
	10 timed, one shot
	11 timed, retriggerable
	bit 2 set if dst I/O
	bit 3 set if src I/O
	bit 4 set if DMAPROBE is dst
	5-7: 0

r A1 - src	src mem or I/O
r A2 - dst	dst mem or I/O
r A3 - pre	timing - 28MHz/scaler for mem/mem, 3.5/scaler for any I/O
r A4 - len	size
r A5 - prob	transfer addr
r A6 - stat	status (byte) bit 7 1 = prob has been reached, clear on read

