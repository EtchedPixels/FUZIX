Terminal Interface
==================

Fuzix provides a subset of the standard POSIX termios terminal
interface.

+-----------------------+-----------------------+-----------------------+
| Speed setting         | Y                     | Not all hardware      |
|                       |                       | supports this.        |
|                       |                       | Input/output speed    |
|                       |                       | must match            |
+-----------------------+-----------------------+-----------------------+
| VMIN                  | Y                     | Minimum characters to |
|                       |                       | complete I/O          |
+-----------------------+-----------------------+-----------------------+
| VEOF                  | Y                     | Configurable EOF      |
|                       |                       | character             |
+-----------------------+-----------------------+-----------------------+
| VTIME                 | Y                     | Timeout on input      |
+-----------------------+-----------------------+-----------------------+
| VEOL                  | Partial               | Configurable end of   |
|                       |                       | line character        |
+-----------------------+-----------------------+-----------------------+
| VERASE                | Y                     | Configurable erase    |
|                       |                       | character             |
+-----------------------+-----------------------+-----------------------+
| VINTR                 | Y                     | Configurable          |
|                       |                       | interrupt character   |
|                       |                       | delivering SIGINT     |
+-----------------------+-----------------------+-----------------------+
| VKILL                 | Y                     | Configurable clear    |
|                       |                       | input character       |
+-----------------------+-----------------------+-----------------------+
| VQUIT                 | Y                     | Configurable          |
|                       |                       | interrupt character   |
|                       |                       | delivering SIGQUIT    |
+-----------------------+-----------------------+-----------------------+
| VSTART                | Y                     | Configurable flow     |
|                       |                       | control characters    |
+-----------------------+-----------------------+-----------------------+
| VSTOP                 | Y                     | Configurable flow     |
|                       |                       | control characters    |
+-----------------------+-----------------------+-----------------------+
| VSUSP                 | N                     | No job control        |
+-----------------------+-----------------------+-----------------------+
| VDSUSP                | N                     | No job control        |
+-----------------------+-----------------------+-----------------------+
| VLNEXT                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| VDISCARD              | Y                     | Discard output        |
+-----------------------+-----------------------+-----------------------+
| BRKINT                | N                     | No break support      |
+-----------------------+-----------------------+-----------------------+
| ICRNL                 | Y                     | Map carriage return   |
|                       |                       | to newline on input   |
+-----------------------+-----------------------+-----------------------+
| IGNBRK                | N                     | No break support      |
+-----------------------+-----------------------+-----------------------+
| IGNCR                 | Y                     | Ignore carriage       |
|                       |                       | return on input       |
+-----------------------+-----------------------+-----------------------+
| INLCR                 | Y                     | Map newline to        |
|                       |                       | carriage return on    |
|                       |                       | input                 |
+-----------------------+-----------------------+-----------------------+
| INPCK                 | N                     | No parity support     |
+-----------------------+-----------------------+-----------------------+
| ISTRIP                | Y                     | Strip parity bit      |
+-----------------------+-----------------------+-----------------------+
| IUCLC                 | N                     | Remap upper/lower     |
|                       |                       | case                  |
+-----------------------+-----------------------+-----------------------+
| IXANY                 | N                     | XON/XOFF flow control |
|                       |                       | not supported         |
+-----------------------+-----------------------+-----------------------+
| IXOFF                 | N                     | XON/XOFF flow control |
|                       |                       | not supported         |
+-----------------------+-----------------------+-----------------------+
| PARMRK                | N                     | No parity support     |
+-----------------------+-----------------------+-----------------------+
| IXON                  | N                     | XON/OFF flow control  |
|                       |                       | not supported         |
+-----------------------+-----------------------+-----------------------+
| OPOST                 | Y                     | Output post           |
|                       |                       | processing            |
+-----------------------+-----------------------+-----------------------+
| OLCUC                 | N                     | Map upper/lower case  |
+-----------------------+-----------------------+-----------------------+
| ONLCR                 | Y                     | Map newline to        |
|                       |                       | carriage return on    |
|                       |                       | output                |
+-----------------------+-----------------------+-----------------------+
| OCRNL                 | Y                     | Map carriage return   |
|                       |                       | to newline on output  |
+-----------------------+-----------------------+-----------------------+
| ONLRET                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| OFILL                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| NLDLY                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| CRDLY                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| TABDLY                | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| BSDLY                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| VTDLY                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| FFDLY                 | N                     | Delays not supported  |
+-----------------------+-----------------------+-----------------------+
| CSIZE                 | Y                     | Hardware dependent    |
+-----------------------+-----------------------+-----------------------+
| CSTOPB                | Y                     | Hardware dependent    |
+-----------------------+-----------------------+-----------------------+
| CREAD                 | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| PARENB                | Some                  | No general support,   |
|                       |                       | some hardware support |
+-----------------------+-----------------------+-----------------------+
| PARODD                | Some                  | No general support,   |
|                       |                       | some hardware         |
|                       |                       | supported             |
+-----------------------+-----------------------+-----------------------+
| HUPCL                 | Y                     | Needs extensive       |
|                       |                       | testing               |
+-----------------------+-----------------------+-----------------------+
| CLOCAL                | Y                     |                       |
+-----------------------+-----------------------+-----------------------+
| CRTSCTS               | Y                     | No general support,   |
|                       |                       | some hardware support |
+-----------------------+-----------------------+-----------------------+
| CBAUD                 | Y                     | Baud rate setting     |
|                       |                       | supported             |
+-----------------------+-----------------------+-----------------------+
| ECHO                  | Y                     | Echo characters as    |
|                       |                       | typed                 |
+-----------------------+-----------------------+-----------------------+
| ECHOE                 | Y                     | Echo erase characters |
+-----------------------+-----------------------+-----------------------+
| ECHOK                 | Y                     | Echo kill line        |
+-----------------------+-----------------------+-----------------------+
| ECHONL                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| ICANON                | Y                     | Canonical model       |
|                       |                       | supported (but not    |
|                       |                       | always 256 bytes as   |
|                       |                       | specified by POSIX)   |
+-----------------------+-----------------------+-----------------------+
| IEXTEN                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| ISIG                  | Y                     | Signal delivery from  |
|                       |                       | keyboard events       |
+-----------------------+-----------------------+-----------------------+
| NOFLSH                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| TOSTOP                | N                     |                       |
+-----------------------+-----------------------+-----------------------+
| XCASE                 | N                     |                       |
+-----------------------+-----------------------+-----------------------+

The following ioctls are supported

TCGETS, TCSETS, TCSETSW, TCSETSF: Implement the posix termios
tcgetattr/tcsetattr interfaces at kernel level.

TIOCINQ: read the number of pending characters of input

TIOCFLUSH: implements the POSIX tcflush() interfaces

TIOCHANGUP: implements the user level vhangup() interface

TIOCOSTOP, TIOCOSTART: implement the POSIX tcflow interface.

Kernel Interface
----------------

**int tty_open(uint_fast8_t minor, uint16_t flag)**

This method handles the opening of a tty device. On success it will call
into tty_setup as provided by the platform and then wait for the carrier
if appropriate. tty_setup can be used to handle things like power
management of level shifters.

**int tty_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)**

This method implements the normal read behaviour for a terminal device
and is generally called directly as the device driver method. It will
call into the platform code in order to implement the low level
behaviour.

**int tty_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag)**

This method implements the normal write behaviour for a terminal device
and is generally called directly as the device driver method. It will
call into the platform code in order to implement the low level
behaviour.

**int tty_ioctl(uint_fast8_t minor, uarg_t request, char \*data)**

Implements the standard tty and line discipline ioctls. This can be
wrapped by platform specific code to provide additional ioctl
interfaces, or by other higher layers (see Virtual Terminal) for the
same purpose.

**tty_close(uint_fast8_t minor)**

Called on the last close of the open tty file handle. This can be
wrapped by drivers that need to reverse some action taken in tty_setup.

Driver Helper Functions
-----------------------

**int tty_inproc(uint_fast8_t minor, unsigned char c)**

Queue a character to the tty interface indicated. This method is
interrupt safe. This may call back into the platform tty output code in
order to implement echoing of characters.

**void tty_outproc(uint_fast8_t minor)**

When implementing asynchronous output indicates that more data may be
queued to the hardware interface. May be called from an interrupt
handler. Any output restarted will restart fom a non interrupt context
after the IRQ completes.

**tty_putc_wait(uint_fast8_t minor, unsigned char c)**

Write a character to the output device, waiting if necessary. Not
interrupt safe.

**tty_carrier_drop(uint_fast8_t minor)**

Indicate that the carrier line on this port has been dropped. Can be
called from an interrupt.

**tty_carrier_raise(uint_fast8_t minor)**

Indicate that the carrier line on this port has been raised. Can be
called from an interrupt.

Defines Provided By The Platform
------------------------------------

**NUM_DEV_TTY:** The number of actual tty device ports (one based not zero based)

**TTYDEV:** The device used for the initial init process and for boot prompts

Structures Provided By The Platform
-----------------------------------

**unsigned char [TTYSIZ]**

One buffer per tty instance.

**struct s_queue[NUM_DEV_TTY+1]**

One queue per tty plus queue 0 which is not used. The structure is set
up as

{ buffer, buffer, buffer, TTYSIZE, 0, TTYSIZE / 2 }

Methods Provided By The Platform
------------------------------------

**void tty_putc(uint_fast8_t minor, unsigned char c)**

Write a character to the tty device. Non-blocking. If the device is busy
drop the byte. When handling echo on a particularly primitive port it
may be advantageous to implement a one byte buffer in the driver code.

**int tty_writeready(uint_fast8_t minor)**

Report whether the device can accept a byte of data. This should return
either TTY_READY_NOW (you may write), TTY_READY_SOON (polling may be
used) or TTY_READY_LATER (there is no point polling). The use of
TTY_READY_SOON allows slow platforms to avoid the continuous overhead of
terminal interrupts, or deferring writes until the next timer tick. The
kernel will poll until the process would naturally be pre-empted. On
fast devices it may be worth always returning TTY_READY_LATER. When the
port is blocked due to a long standing condition such as flow control
TTY_READY_LATER should be returned.

**void tty_sleeping(uint_fast8_t minor)**

This method is called just before the kernel exits polling of a tty
port. This allows the driver to selectively enable transmit complete
interrupts in order to reduce CPU loading. For other platforms this may
be a null function.

**int tty_carrier(uint_fast8_t minor)**

Report the carrier status of the port. If the port has no carrier
control then simply return 1 (carrier present).

void tty_setup(uint_fast8_t minor)

Perform any hardware set up necessary to open this port. If none is
required then this can be a blank function.

**void kputchar(char c)**

Writes a character from the kernel to a suitable console or debug port.
This is usually implemented in terms of tty_putc. As it may be called
from an interrupt it cannot use tty_putc_wait(). On platforms with queued
interrupt driven output this routine should ideally not return until the
character is visible to the user.

Implementation Models
---------------------

Generally speaking the terminal interfaces found on 8bit systems split
into three categories beside built in display and keyboard: bit banged
ports, polled serial ports and interrupt driven serial ports. Bit banged
ports pose a particularly difficult set of problems with a multitasking
operating system.

If the platform has polled serial ports then the port should be polled
for input and carrier changes on the timer tick. To get the best
responsiveness it is strongly recommended that the port is also polled
from platform_idle(). As the timer tick can occur during platform_idle()
care must be taken to lock out any re-entrancy of the polling logic. Any
FIFOs should be emptied in the poll so that the chance of overruns
before the next read is minimised.

Interrupt driven ports can queue characters directly from the events as
they occur. This provides a much better experience. Transmit complete
interrupts can also be used but are much less effective on low speed
devices as the time taken to complete interrupt handling, wake and begin
queueing a further byte will exceed the time taken to actually send a
byte of output. Whether they are beneficial depends upon the platform. The
kernel tty layer is deliberately designed to be unaware of whatever low
level interrupt driven queued I/O or polled I/O is happening.

Bit-bang ports are problematic. Transmit is relatively easy as the
tty_putc() method can disable interrupts if not already off and simply
bash out the data. There is a performance impact but this is otherwise
fairly reliable. Receive is very difficult to arrange as the system will
be carrying out other tasks. At best it could be polled when the machine
is idle.

A small number of bitbang interfaces either generate interrupts on
edges, or timestamp them. These are ultimately much the same as a normal
interrupt driven port minus the small matter of coding and the need for
a very fast specialised interrupt handler for the bit banging. This
probably needs to occur in assembler without calling the standard
interrupt entry paths.

Bugs And Caveats
----------------

The tty layer is one of the few parts of the kernel that makes
active use of interrupts. Driver code therefore needs to be careful that
the interrupt handling is race free when testing conditions changed by
the interrupt handlers.

Parity can be handled in hardware but parity checking and generation in
software are not currently supported, even though they would be very
cheap to do on Z80 at least.

Carrier handling is very new and not well tested.

