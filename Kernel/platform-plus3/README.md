`A FUZIX target for ZX Spectrum +2A/+3`

The +2A and +3 have the following choice of memory configurations on top of
the standard 128K spectrum

```
	00  40  80  C0
conf0	[0] [1] [2] [3]
conf1	[4] [5] [6] [7]
conf2	[4] [5] [6] [3]
conf3	[4] [7] [6] [3]
```

We have an interesting limitation: unlike the 128K spectrum banks 4-7 are
all contended (slower). In addition pointing I into those banks will crash
the machine. Providing we keep RAM in the low 16K we can avoid IM2 however.

That gives us a conventional low 0/1 and 4/5 for user space with the kernel
using 3/6/7. Given the memory napping limits it actually probably makes
more sense to eventually allow one 48K and one 32K mapping so most processes
can run two at a time but one big one is allowed. That implies we get our
chmem act together.

We then map

0/1/2/3	-	user process 0: 48K, 16K common
4/5/6/3 -	user process 1: 32K, 16K common, 16K is kernel reserved
4/7/6/3 -	kernel: 48K minus screen, common 16K (some kernel in it)
		(conveniently the map 'BOOT' uses)

As an aside this only works for the +2A/+3. The various clones even with
lots of RAM clone the original 128K and add more pages of memory in the top
16K bank. Some allow the low 16K to become RAM (eg for CP/M) but have
4000-BFFF fixed.

There are also various add in boards some of which can work on the +2A/+3
that provide alternative RAM and ROM banks for the low 16K. We don't make
use of them in this configuration althogh in future it would make sense to
make the kernel mapping in such a case map DivMMC or similar RAM into the
low 16K when in kernel mapping so the kernel becomes mmc/7/3 and we leave 6
free to allow a pair of 48K processes.
