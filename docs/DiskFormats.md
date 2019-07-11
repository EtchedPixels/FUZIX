# Proposed Fuzix Standard Floppy Disk Formats

## 8" Drives

- Single Density
26 sectors per track, 128 bytes per sector soft skew
of 6. IBM 3740 format, standard CP/M format. May be hard
sectored. Usually 77 tracks. Defacto world wide standard
for 8" single density.

- Duble Density
  16 sectors per track, 512 bytes per sector,  skewed at format
  time. (Cromemco and others).

## 5.25" Drives

These may be 35, 40, 77 or 80 track. Doublestep may be needed.

- Single Density
  18 sectors per track, 128 bytes per sector, skewed at format
  time or do we do  9 / 256 ?.

- Double Density
  10 sectors per track, 512 bytes per sector, skewed at format
  time. Must also support 9 sectors/track (IBM PC format).

- High Density
  15 sectors per track, 512 bytes per sector, skewed at format
  time. (IBM PC format).

## 3.5" Drives

- Double Density
  As 5.25"

- High Density
  18 sectors per track, 512 bytes per sector, skewed at format
  time. (IBM PC format)

- EHD
  18 sectors per track, 512 bytes per sector, skewed at format
  time. (IBM PC format)

## 3" Drives

These may be 40 or 80 track, single or double sided. Single sided disks are
flippable. They are all double density.

- Double Density
  9 sectors per track, 512 bytes/sector, skewed at format time.
  (Amstrad format).

Flippable media can either be treated as one file system (only readable properly
on a double sided drive), or two. Double step may be needed

## General Rules

Systems should support physical media formats used by other common OS on their
platform, even if not auto-detected. Floppy media should not be partitioned.
Logical sector order is sector/side/track. In other words blocks are ordered on
track 0 side 0, then track 0 side 1, then track 1 side 0 and so forth.

Platforms that mix 40 and 80 track media should support double step if possible.

Some platforms have their own way of doing things so will need a different
private formatting default. They should if possible also support something
standard. This affects Apple and Commodore mostly.
