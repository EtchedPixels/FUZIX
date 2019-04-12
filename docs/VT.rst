Virtual Terminal
================

The vt interface layer provides a generalized VT52 like console for
systems with a built in display. It is not used when only serial ports
are present.

Virtual Terminal Codes
----------------------

+-----------------------------------+-----------------------------------+
| Ctrl-G                            | Sounds the beeper if a beep       |
|                                   | method is provided                |
+-----------------------------------+-----------------------------------+
| Ctrl-H                            | Moves the cursor back one         |
|                                   | character                         |
+-----------------------------------+-----------------------------------+
| Ctrl-I                            | Moves forward to the next tab     |
|                                   | stop (8 character fixed stops)    |
+-----------------------------------+-----------------------------------+
| Ctrl-J                            | Move the cursor down a line,      |
|                                   | scroll the screen if at the       |
|                                   | bottom                            |
+-----------------------------------+-----------------------------------+
| Ctrl-M                            | Move the cursor back to column    |
|                                   | zero on the current line          |
+-----------------------------------+-----------------------------------+
| Escape                            | Introduce an escape sequence      |
+-----------------------------------+-----------------------------------+
| Escape-A                          | Move the cursor up a line if not  |
|                                   | on the top line                   |
+-----------------------------------+-----------------------------------+
| Escape-B                          | Move the cursor down a line if    |
|                                   | not on the bottom line            |
+-----------------------------------+-----------------------------------+
| Escape-C                          | Move the cursor right a character |
|                                   | if not on the right hand edge     |
+-----------------------------------+-----------------------------------+
| Escape-D                          | Move the cursor left a character  |
|                                   | if not on the left hand edge      |
+-----------------------------------+-----------------------------------+
| Escape-E                          | Clear the screen, the cursor      |
|                                   | position is unchanged             |
+-----------------------------------+-----------------------------------+
| Escape-H                          | Move the cursor to the top left   |
+-----------------------------------+-----------------------------------+
| Escape-I                          | Move the cursor up, if already at |
|                                   | the top then reverse scroll the   |
|                                   | screen                            |
+-----------------------------------+-----------------------------------+
| Escape-J                          | Clears from the cursor position   |
|                                   | to the end of screen, the cursor  |
|                                   | position is unchanged             |
+-----------------------------------+-----------------------------------+
| Escape-K                          | Clears from the cursor position   |
|                                   | to the end of line, the cursor    |
|                                   | position is unchanged             |
+-----------------------------------+-----------------------------------+
| Escape-Y                          | Introduce a movement sequence.    |
|                                   | The next two bytes are            |
|                                   | interpreted as the X and Y        |
|                                   | position where space (32) is 0.   |
|                                   | An attempt to move off screen on  |
|                                   | one axis causes that axis to      |
|                                   | remain unchanged, but does not    |
|                                   | stop the movement occuring in the |
|                                   | other axis                        |
+-----------------------------------+-----------------------------------+
| Escape-a c                        | Set the attributes if supported   |
+-----------------------------------+-----------------------------------+
| Escape-b c                        | Set the ink (foreground) colour   |
|                                   | that is used in rendering the     |
|                                   | text mode display                 |
+-----------------------------------+-----------------------------------+
| Escape-c c                        | Set the paper (background) colour |
|                                   | that is used in rendering the     |
|                                   | text mode display                 |
+-----------------------------------+-----------------------------------+ 

The text mode colour codes are defined as a best match to IRGB colours
(intense, red, green, blue) with the IRGB forming the lower 4bits of the
code.

The other bits are used as follows

4: reserved (must be zero if bit 5 is zero)

5: platform specific colour not standard IRGB (colour is then bits 4-0)

6: always 1 to keep the code in the ASCII range

7: always 0 to keep the code in the ASCII range

There is currently no facility to set border colours.


Virtual Terminal Ioctls
-----------------------

+-----------------------------------+-----------------------------------+
| KBMAPSIZE                         | Reports the keyboard map size.    |
|                                   | The number of rows is in the high |
|                                   | byte of the return. The number of |
|                                   | columns in the low byte. Not all  |
|                                   | keyboards are row based so this   |
|                                   | ioctl is not always supported.    |
+-----------------------------------+-----------------------------------+
| KBMAPGET                          | Returns the current keymap with   |
|                                   | set bits indicating the keys      |
|                                   | currently down, within the limits |
|                                   | of the hardware. KBMAPSIZE        |
|                                   | determines the size of the map as |
|                                   | rows are packed into 16bit words  |
+-----------------------------------+-----------------------------------+
| KBSETTRANS                        | Set the translation table for     |
|                                   | this keyboard. The translation    |
|                                   | tables are machine specific and   |
|                                   | the ioctl data must be in the     |
|                                   | expected format for the system.   |
|                                   | Only root may use this interface  |
+-----------------------------------+-----------------------------------+
| VTSIZE                            | Reports the height of the         |
|                                   | terminal (in characters) in the   |
|                                   | upper byte and the width in the   |
|                                   | lower byte.                       |
+-----------------------------------+-----------------------------------+

Kernel Programming Interface
----------------------------

Most of the VT features are configured in the platform config.h

+-----------------------------------+-----------------------------------+
| CONFIG_VT                         | Builds the VT layer into the      |
|                                   | kernel                            |
+-----------------------------------+-----------------------------------+
| CONFIG_VT_SIMPLE                  | Builds the supporting code for a  |
|                                   | simple character based memory     |
|                                   | mapped display                    |
+-----------------------------------+-----------------------------------+
| CONFIG_UNIKEY                     | Include support for a few unicode |
|                                   | symbols on input                  |
+-----------------------------------+-----------------------------------+
| VT_WIDTH                          | Width of the display in           |
|                                   | characters                        |
+-----------------------------------+-----------------------------------+
| VT_HEIGHT                         | Height of the display in          |
|                                   | characters                        |
+-----------------------------------+-----------------------------------+
| VT_RIGHT                          | Usually VT_WIDTH – 1 unless the   |
|                                   | display has a wider stride        |
+-----------------------------------+-----------------------------------+
| VT_BOTTOM                         | VT_HEIGHT – 1                     |
+-----------------------------------+-----------------------------------+
| VT_BASE                           | Used with CONFIG_VT_SIMPLE, gives |
|                                   | the memory address that forms the |
|                                   | start of the display              |
+-----------------------------------+-----------------------------------+
| VT_MAP_CHAR(x)                    | Used with CONFIG_VT_SIMPLE to     |
|                                   | remap from ASCII to the platform  |
|                                   | native symbols                    |
+-----------------------------------+-----------------------------------+
| VT_INITIAL_LINE                   | Optional (defaults to zero),      |
|                                   | allows the display to start at    |
|                                   | boot further down the screen.     |
|                                   | This can be useful to preserve    |
|                                   | boot loader messages.             |
+-----------------------------------+-----------------------------------+

The core vt services are designed to be used from a tty driver and to
stack on top of the existing tty interface. The vt layer provides the
following methods

**void vtoutput(unsigned char \*p, unsigned int len)**

Output one or more characters to the VT52 screen while performing
terminal emulation

**int vt_ioctl(uint_fast8_t minor, uarg_t request, char \*data)**

Replaces tty_ioctl and implements the vt layer interfaces before calling
on into tty_ioctl

**void vt_inproc(uint_fast8_t minor, unsigned char c)**

Wraps tty_inproc for vt devices and should be used instead of tty_inproc

**void vtinit(void)**

Initializes the VT interface, should be called before **fuzix_main()** is
entered, so usually from init_hardware.

.. _methods-provided-by-the-platform-1:

Methods Provided By The Platform
--------------------------------

When not using CONFIG_VT_SIMPLE the platform must provide a set of
methods to manipulate the display memory.

The methods required are

**void do_beep(void)**

Make a beep noise, or perform a visual bell (eg inverting the display
momentarily if the hardware supports it).

**void cursor_disable(void)**

Disable the cursor even if it is a hardware cursor. This is used to indicate
that the user has specifically disabled the cursor, rather than indicating a
request to hide the cursor for a display update.

**void cursor_off(void)**

Hide the cursor, whether in hardware or software. If necessary restore any
characters or attributes changed by cursor_on(). This routine can be a no-op
if there is a hardware cursor and updating the display does not affect it.

**void cursor_on(int8_t x, int8_t y)**

Turn the cursor back on at x,y. Ensure that any data that is needed to
restore the display is saved so that cursor_off() can use it.

The following methods will only be invoked between a cursor_off() and a
cursor_on()

**void clear_across(int8_t y, int8_t x, int16_t num)**

Clear num characters across from y,x. The request will never span more than
the one line. The normal case is from x to the right hand side of the
display. A request to clear zero characters should be handled.

**void clear_lines(int8_t y, int8_t num)**

Clear num lines starting at line y. They are cleared for the full width for
the display. A request to clear zero lines should be handled.

**void scroll_up(void)**

Scroll the screen up (normal scroll). The bottom line does not need to
be cleared. Hardware scrolling can be used if present.

**void scroll_down(void)**

Scroll the screen down (reverse scroll). The top line does not need to
be cleared. Hardware scrolling can be used if present.

**void plot_char(int8_t y, int8_t x, uint16_t c)**

Plot a character onto the display at y,x. Currently c is in the range 0-255
but that may change in future to accomodate non-ascii displays and systems with
additional graphic ranges.

Fonts
-----

Bitmap displays usually require a font in the kernel. There are several
provided fonts that can be compiled into the kernel using config.h
defines

**CONFIG_FONT_8X8:** A classic 8x8 pixel font for 256 symbols.

**CONFIG_FONT_8X8SMALL:** A 96 symbol version of the 8x8 font for
tight spaces. Note that this font is character 32 based not character 0
based.

**CONFIG_FONT_6X8:** A 6x8 pixel font with the bits packed to the right.
Used to get 40 characters on a 256 pixel wide display.

**CONFIG_FONT_4X6:** A 4x6 pixel font for awkward displays. Each symbol
is replicated in the high and low nybbles of the data stream in order to
optimize rendering.

**CONFIG_FONT_4X8** A 4x8 pixel font for narrow displays. Each symbol
is replicated in the high and low nybbles of the data stream in order to
optimize rendering.

**CONFIG_FONT_8X8_EXP2** A 16bit wide font where each character has been
expanded to two adjacent bits. Used for rendering onto 4 colour packed
bitmap displays such as the Sam Coupe.

All the fonts are linked into the FONT section of the kernel image. This
is done because on some platforms the rendering code is in common space
and it is desirable to arrange that the FONT is mapped in a location
that is accessible when the display is mapped, but not necessarily
wasting COMMON space. On certain platforms the FONT area may also be
discardable if the font is loaded into hardware at boot time.

Keyboard
--------

The second half of the VT driver is the keyboard interface. This is
mostly platform specific code. The kernel tries to abstract the
interface to this so that keydown and keyup data is provided in a
standardized bitmap of 16bit words, with each row mapped to a number of
words.

To aid standardization a set of standard keyboard codes are defined.
Those outside of the ASCII set will be translated by the kernel into
unicode providing **CONFIG_UNIKEY** is defined. The codes given below
also have their top bit set except for control key codes and delete

+-----------------------+-----------------------+-----------------------+
| 0x7F                  | KEY_DEL               | Delete. Prefer        |
|                       |                       | backspace for normal  |
|                       |                       | delete                |
+-----------------------+-----------------------+-----------------------+
| Ctrl-H                | KEY_BS                | Backspace (delete     |
|                       |                       | left)                 |
+-----------------------+-----------------------+-----------------------+
| Ctrl-[                | KEY_ESC               | Escape key            |
+-----------------------+-----------------------+-----------------------+
| Ctrl-I                | KEY_TAB               | Tab key               |
+-----------------------+-----------------------+-----------------------+
| Ctrl-J                | KEY_ENTER             | The enter key on a    |
|                       |                       | keyboard, or other    |
|                       |                       | key serving this      |
|                       |                       | purpose               |
+-----------------------+-----------------------+-----------------------+
| Ctrl-L                | KEY_CLEAR             | Clear button on a     |
|                       |                       | keyboard              |
+-----------------------+-----------------------+-----------------------+
| D                     | KEY_LEFT              | Left arrow key (not   |
|                       |                       | backspace)            |
+-----------------------+-----------------------+-----------------------+
| C                     | KEY_RIGHT             | Right arrow key (not  |
|                       |                       | delete right)         |
+-----------------------+-----------------------+-----------------------+
| A                     | KEY_UP                | Up arrow key          |
+-----------------------+-----------------------+-----------------------+
| B                     | KEY_DOWN              | Down arrow key (not   |
|                       |                       | newline/carriage      |
|                       |                       | return)               |
+-----------------------+-----------------------+-----------------------+
| H                     | KEY_HOME              | Home button           |
+-----------------------+-----------------------+-----------------------+
| h                     | KEY_HELP              | Help button           |
+-----------------------+-----------------------+-----------------------+
| I                     | KEY_INSERT            | Insert button         |
+-----------------------+-----------------------+-----------------------+
| c                     | KEY_COPY              | Copy button           |
+-----------------------+-----------------------+-----------------------+
| v                     | KEY_PASTE             | Paste button          |
+-----------------------+-----------------------+-----------------------+
| x                     | KEY_CUT               | Cut button            |
+-----------------------+-----------------------+-----------------------+
| a                     | KEY_CANCEL            | Cancel button or      |
|                       |                       | similar               |
+-----------------------+-----------------------+-----------------------+
| e                     | KEY_EXTRA             | Extra button          |
+-----------------------+-----------------------+-----------------------+
| ]                     | KEY_PRINT             | Print button          |
+-----------------------+-----------------------+-----------------------+
| Ctrl-C                | KEY_STOP              | Stop/Halt/Interrupt   |
|                       |                       | button (not the       |
|                       |                       | normal ^C key map)    |
+-----------------------+-----------------------+-----------------------+
| r                     | KEY_DELR              | Delete right          |
+-----------------------+-----------------------+-----------------------+
| +                     | KEY_PLUS              | Plus button (not      |
|                       |                       | normal '+')           |
+-----------------------+-----------------------+-----------------------+
| -                     | KEY_MINUS             | Minus button (not     |
|                       |                       | normal '-')           |
+-----------------------+-----------------------+-----------------------+
| q                     | KEY_EXIT              | Exit/Quit button      |
+-----------------------+-----------------------+-----------------------+
| p                     | KEY_PAUSE             | Pause button          |
+-----------------------+-----------------------+-----------------------+
| <                     | KEY_PGDOWN            | Page up               |
+-----------------------+-----------------------+-----------------------+
| >                     | KEY_PGUP              | Page down             |
+-----------------------+-----------------------+-----------------------+
| E                     | KEY_EDIT              | Edit button           |
+-----------------------+-----------------------+-----------------------+
| N                     | KEY_END               | End button (not stop  |
|                       |                       | but move to line end) |
+-----------------------+-----------------------+-----------------------+
| U                     | KEY_UNDO              | Undo button           |
+-----------------------+-----------------------+-----------------------+
| 1                     | KEY_F1                | Function key 1        |
+-----------------------+-----------------------+-----------------------+
| 2                     | KEY_F2                | Function key 2        |
+-----------------------+-----------------------+-----------------------+
| 3                     | KEY_F3                | Function key 3        |
+-----------------------+-----------------------+-----------------------+
| 4                     | KEY_F4                | Function key 4        |
+-----------------------+-----------------------+-----------------------+
| 5                     | KEY_F5                | Function key 5        |
+-----------------------+-----------------------+-----------------------+
| 6                     | KEY_F6                | Function key 6        |
+-----------------------+-----------------------+-----------------------+
| 7                     | KEY_F7                | Function key 7        |
+-----------------------+-----------------------+-----------------------+
| 8                     | KEY_F8                | Function key 8        |
+-----------------------+-----------------------+-----------------------+
| 9                     | KEY_F9                | Function key 9        |
+-----------------------+-----------------------+-----------------------+
| 0                     | KEY_F10               | Function key 10       |
+-----------------------+-----------------------+-----------------------+
| :                     | KEY_F11               | Function key 11       |
+-----------------------+-----------------------+-----------------------+
| ;                     | KEY_F12               | Function key 12       |
+-----------------------+-----------------------+-----------------------+
| 0x80                  | KEY_POUND             | Remapped to the       |
|                       |                       | unicode for a '£'     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+
| 0x81                  | KEY_HALF              | Remapped to the       |
|                       |                       | unicode for a '½'     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+
| 0x82                  | KEY_EURO              | Remapped to the       |
|                       |                       | unicode for a '€'     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+
| 0x83                  | KEY_DOT               | Remapped to the       |
|                       |                       | unicode for a dot     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+
| 0x84                  | KEY_YEN               | Remapped to the       |
|                       |                       | unicode for a '¥'     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+
| 0x85                  | KEY_COPYRIGHT         | Remapped to the       |
|                       |                       | unicode for a '©'     |
|                       |                       | sign                  |
+-----------------------+-----------------------+-----------------------+

Multiple Console Support
------------------------

Some platforms either have two or more monitors, or have enough video
memory to support multiple switchable consoles. In order to keep the
code size small for the default case the VT switching is handled as a
special case. If CONFIG_VT_MULTI is defined then two helper methods are
supplied

**void vt_save(struct vt_switch \*vt)**

Saves the VT layer state for the current active console

**void vt_load(struct vt_switch \*vt)**

Load back the VT layer state for the saved consoles

If the console sizes are the same (for example when implementing virtual
consoles) the usual defines can be maintained if not then VT_WIDTH and
the related defines can be defined as array references.

The implementation needs to track a current output tty and a current
input tty. The tty_putc() method should vt_save the existing tty and
switch to the new tty (based upon minor) and then load its state with
vt_load() before calling vtoutput(). Currently this needs to be done
with interrupts disabled. Fixing that limit is planned for the future.

The output helpers provided by the platform can check curtty in order to
decide where to render or which methods should be invoked.

Input shifting is in the hands of the platform. It needs to invoke the
inproc method for the current virtual console. How it decides which
console to use is up to the kernel. On a system with a single keyboard
and multiple console this could be done by reserving keys for switching.
Current platforms use shift-F1 shift-F2 .. for console switching.

For a worked example of multiple console support see the Memotech MTX
platform.

