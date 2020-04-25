# Floppy Disks

## Introduction

Fuzix treats a floppy disk as a classic Unix style block device. It contains
a series of logical 512 byte sectors numbered from 0 upwards. The hardware
geometry of the interface and media is abstracted away.

Unlike most disk devices floppy drives are often used with multiple media
formats, and also have additional operations such as formatting. These are
supported by additional ioctls.

For actual structures see Kernel/include/fdc.h

## Mandatory Ioctl Interfaces

### FDIO_GETCAP

This returns a structure giving them union of all supported feature sets
available on this interface. In addition the mode value gives the current
mode.

### FDIO_GETMODE

This returns the full structure for the mode passed in. A simple disk
interface may only have a single mode, in which case this can be implemented
as a single check against zero and falling into FDIO_GETCAP. An application
can enumerate disk modes by quering from mode zero upwards until it receives
an error. All modes will be sequential from zero without gaps.

## Optional Interfaces

### FDIO_SETMODE

Set the desired mode to one of the offered entries in the mode table. This
change is persistent until the mode is changed again. A device with only one
mode need not implement this function.

### FDIO_FMTTRK

TBD

### FDIO_RESTORE

Seek the disk head back to track zero or another safe place.

### FDIO_SETSKEW

Applications can pass a 32 byte table to set the skew table on drives that
are not soft skewed or need to support both modes. Bytes beyond the number
of sectors per track are not used. Byte zero corresponds to the first sector
and so on.

### FDIO_SETSTEP

Set the step, headload and settle times for the drive. These are normally
part of the mode but can be set separately in some cases where special
allowances may be required.

## Design Concept

The idea of the disk modes is to keep the code size down. Most systems have
very few meaningful disk configurations. A table driven approach allows the
drivers to simply keep a precomputed configuration table for the actual
floppy controller rather than handle all possible cases.

Consideration should be taken of both standard modes and traditional modes
for that platform. All systems should support the modes they can from the
classic PC 360K/720K/1.44MB settings even if they are not traditionally used
on those systems. This aids hugely in media compatibility.
