This is a work-in-progress port to the NC200 which boots (and hopefully runs)
from floppy (using an SRAM card).

To use, format a disk (preferably on the NC200 itself, as the interleave
settings used by the NC200 will cause it to load much more quickly), and do:

```
cd Kernel
make TARGET=nc200
dd if=fuzixfloppy.img of=/dev/fd0 bs=1k
```

Insert the disk into the NC200, turn on, and press Function+R. The kernel
will be loaded from floppy, programmed into the SRAM card, and run.

The current state is that there's just enough working to display the Fuzix
startup banner, most times. It's certainly not suitable for real use. Your
kilometreage may vary.