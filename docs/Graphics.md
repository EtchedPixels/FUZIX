# Graphics

*WIP* everything may change

## Ioctl interface

### GFXIOC_GETINFO

Obtains the current video settings for the display. This call will fail if
the display does not support graphics.

### GFXIOC_MAP

Map the video display into the application memory or I/O space. On a
protected mode system this may be MMU based. On many 8bit microcomputers
without protection the display is already in the address space and the
relevant data is provided. In a few cases it may involve changing mappings.

This call will generally also stop console output to the display and prevent
screen switching where present.

This function is optional. When not present indirect drawing methods must be
used.

### GFXIOC_UNMAP

Undo the mapping acquired and restore the display. Safe to call if you don't
have a mapping.

### GFXIOC_WAITVB

Wait for end of frame or vertical blank to begin. Optional.

## Monochrome Bitmap Details

*FMT_MONO_BW* denotes a bit per pixel display with the highest bit on the
left. Each row is formed of sequential byte packed pixels and each line
start is separated by the stride given in the display structure. A set bit
indicates a black pixel.

The underlying video space is lines x stride bytes in size although the
visible area may be constrained and not scrollable.

*FMT_MONO_WB* denotes the equivalent display but a set bit denotes a white
pixel.

### Four Colour Packed Pixels

*FMT_COLOUR4* denotes a two bit per pixel display packed with the highest
bits on the left. Each row is formed of sequential byte packed two bit
colour codes. The colours can be obtained by quering the palette and may
sometimes be settable. Each line start is separated by the stride given in
the display structure.

The underlying video space is lines x stride bytes in size although the
visible area may be constrained and not scrollable.


TODO: what about mode dependent colour ? (eg CGA)

### Sixteen Colour Packed Pixels

*FMT_COLOUR16* denotes a four bit per pixel display packed with the highest
bits on the left. Each row is formed of sequential byte packed four bit
colour codes. The colours can be obtained by quering the palette and may
sometimes be settable. Each line start is separated by the stride given in
the display structure.

The underlying video space is lines x stride bytes in size although the
visible area may be constrained and not scrollable.

## Text Only

*FMT_TEXT* denotes a text only mode where the display can only be character
addressed and each line is formed of left to right bytes of character
symbol data, possibly with limited range. Each line start is separated by
stride bytes. Only the presence of the ASCII symbols between 32 and 127 is
guaranteed. Where user defined fonts and graphics are also supported the
VTSETUDG interface can be used to define custom symbols

## Sixel Graphics and Text

*FMT_6PIXEL_128* denotes a display with sixel graphic support as well
as the ASCII range given above. The format is indentical but symbols in
the range 128-191 form six block graphics with lowest bit being the top left
block and the highes the bottom right.

[TODO: Add something to deal with upper case only display on either of
these. Also look at other variants we might need to deal with and default
UDG for this mode]

## ZX Spectrum

The ZX Spectrum has a rather unique display layout which is described by
*FMT_SPECTRUM*, and can be found on various ZX Spectrum related and
compatible systems with a mono interleaved bitplane display and a separate
character sized colour plane.

## TMS9918A/9938/9958 VDP

*FMT_VDP* indicates a system with VDP style graphics. This is effectively a
graphics co-processor (especially on the 9938/9958) and has its own interface. The
MAP operation on this device maps the VDP not the framebuffer.

## Microbeee

*FMT_UBEE* denotes the rather strange video of the Microbee series systems.

## SAM2

*FMT_SAM2* denotes the Sam Coupe model 2 (pixel high characters with a
separate atttribute map).

## TIMEX64

*FMT_TIMEX64* denotes the 64 character (512 pixel) modes used on some
Sinclair Spectrum related systems. The display consists of two byte
interleaved ZX Spectrum format screens.

## TMS9918A Inteface Specific Details

The TMS9918A interface permits an application to configure and render to
the VDP. If rendering directly via GFXIOC_MAP the lowest level routines will
need to manage interrupts, but this ugly will only apply to unprotected 8bit
systems.

The TMS9918A driver in the kernel reserves the 3C00-3FFF range in the video
memory for the font cache used during video restore after graphics. This
range must not be used by application, and is not needed in any mode.

In modes except Graphics II where the space has to be used the application
should avoid using the area 0000-0FFF as this will keep the console display
contents preserved.

The TMS9918A interface is currently only provided for Z80 RC2014 based
systems.

Do not assume the VDP is at address 0x98/99 - on many non-MSX systems this
is simply not the case.

### VDPIOC_SETUP

This ioctl is passed a table of register settings for the TMS9918A
of the map for that device. The VDP registers are then loaded after
modification by the OS to keep various status bits it needs correct

### VDPIOC_WRITE (not yet done)

This ioctl is passed a pointer to a descriptor giving a 16bit size (max 1K)
and offset in VDP plus a pointer to the bytes to write. It is intended to be
used on systems where the VDP is not mappable.

### VDPIOC_READ	(not yet done)

Ths ioctl serves the same purposes a VDIOC_WRITE but data is read into the
buffer instead.

We need read/write rectangle as well.

## TRS80 High Resolution Specific Details

The TRS80 had a variety of graphics display accessible via I/O ports. These
need specific drivers that use GFXIOC_MAP to map the I/O ports and use them
to draw graphics. The kernel knows about the HRG1B (Video Genie), and
Microlabs Grafyx variants for the different systems as well as the official
TRS80 model 4 board and very experimentally the Lowe Electronics LE18.

The various TRS80 software downloadable character devices are not handled as
displays but where supported as user defined graphics.

