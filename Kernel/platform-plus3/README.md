`An FUZIX target for ZX Spectrum +2A/+3`

The +2A and +3 have the following choice of memory configurations on top of
the standard 128K spectrum

```
	00  40  80  C0
conf0	[0] [1] [2] [3]
conf1	[4] [5] [6] [7]
conf2	[4] [5] [6] [3]
conf3	[4] [7] [6] [3]
```

That gives us a conventional low 0/1 and 4/5 for user space with the kernel
using 2/3/6/7.

Kernel maps are then

3 = common (always mapped high)
2/6 = banked (at 0x8000)
7 = banked with screen (at 0x4000)

although 7 does not appear to be part of the banks we cannot map 2 and 7
together so it's effectively banked.

Alternatively we could go with a single 64K swapping user space with
kernel mapped normally at 4-7  (with screen hole at C000) and user at 0-3
and a small copied common in bank 3 and 7
