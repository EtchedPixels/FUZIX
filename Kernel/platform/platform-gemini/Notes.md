# Gemini Platform

Gemini GM811 CPU card
- ASCII keyboard port
- Z80 PIO port
- Serial 	8250A @2MHz
- Aux
- Tape

Usually set to reset jump to ROM at F000. Internal memory is
C000/D000/E000/F000 if populated (and subject to RAMDIS)

I/O
B0		keyboard
B4-B7		PIO (AD BD AC BC) IM2
B8-BF		UART

BRG
 09C4		50
 0683		75
 0470		110
 03A1		134.5
 0341		150
 01A1		300
 00D0		600
 0068		1200
 0034		2400
 001A		4800
 000D		9600

Keyboard strobe optionally PIO A bit 0 (optional link)
UART is PIO A bit 1 for IRQ option

UART /OU1 controls serial or tape
/OUT2 controls onboard memory mapping

(setting bit in reg lowers line for OUT* !)
IRQ is active high

As supplied 
C000-FFFF are the bytewide sockets unless disabled. As their may be none we
need thunking

Shipped with RPM
RPM can boot the boot sector of various disk options (809/29/49)

RAM page mode GM002 cards (64K) 0000-EFFF (no video contention)

Option for GM812 video
B1 data
B2 status 0 = write full 7 = read full
0B3 reset

GM816
CTC		0x10-13		IM2
PIO		0x14-17		""
PIO		0x18-1B		""
PIO		0x1C-1F		""
MK3881		0x20-2F		NMI
