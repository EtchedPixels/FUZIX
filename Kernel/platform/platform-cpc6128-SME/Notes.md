# Amstrad CPC6128


## Mmeory Model

This port is based on the zx+3 port, taking advantage of Alan Cox's suggestion to take advantage of the fact that the memory maps C1, C2 and C3 correspond to the maps used in the zx+3 port.

In the zx+3:
| User		| 0 / 1 / 2  | Common 3
| Kernel	| 4 / 5 / 6  | Common 3
| Video		| 4 / 7 / 6  | Common 3

In the CPC6128:
| User		| 4 / 5 / 6  | Common 7 (map C2)
| Kernel	| 0 / 1 / 2  | Common 7 (map C1)
| Video		| 0 / 3 / 2  | Common 7 (map c3)

## TODO

Lots of bugs.
Trying to mount the flopy hangs the machine, and may do nasty things to the floppy and the drive, don't try on a real machine!!
Fix memory size reporting 64 v 48K.