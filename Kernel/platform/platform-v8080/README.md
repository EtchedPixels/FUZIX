# Virtual 8080 Platform Using Z80Pack

## Platform

Z80pack run with cpmim -8 to select 8080 processor. Used to check the status
of the 8080 codebase. We don't currently have any pure 8080 platforms
supported only 8085.

## Installation

make diskimage

Use the boot disk as drivea.dsk and the root disk as drivep.dsk. It will
also use drivej.dsk as swap.

P drive is 7 at the boot prompt
