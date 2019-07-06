# Introduction

## What Is Fuzix

Fuzix is an operating system for eight bit microprocessors, and some bigger
systems that provides a large subset of the traditional 'Unix like'
environment including multi-tasking, the ability to execute multiple
programs at a time.

Fuzix manages your system resources, memory, storage and devices. It
provides a unified programming interface across many different types of
machine, as well as providing a consistent but customisable  user interface.

The hardware requirements for Fuzix are small, although higher than those
for less functional operating systems such as CP/M. The detailed
requirements are enumerated later in the manual as they vary by platform

## How Does Fuzix Differ From Traditional Unix Systems

Fuzix is derived from Uzi which was itself based upon early Unix ideas from
between V6 and V7 Unix platforms. Fuzix extends this to reflect more modern
Posix interfaces for things like terminal handling.

Classic Unix split into two differing strands BSD Unix and SYS5 Unix. Over time
these were unified by putting every feature of both of them into one large and
complex system. Fuzix takes a different approach and tries to pick the
sanest and cleanest interfaces. Generally speaking those are also the ones
that applications use.

In general Fuzix leans towards System III and early System V, but favours
POSIX standardised interfaces where possible. Certain nonsensical System V
misfeatures are deliberately omitted (notably System V IPC). The networking
interfaces are based upon the BSD socket API rather than the System V
streams API that nobody ever uses. File locking is via BSD flock() not the
System V interface because flock is vastly less complex.

The biggest differences are driven by hardware. Most Fuzix systems do not
have an MMU (memory management unit) so Fuzix does not provide the related
family of interfaces (mmap, mprotect, munmap etc). Signals related to memory
protection may also not be generated in hardware but are available at the
software level.

Eight bit systems have relatively small address spaces so programs that are
written for mainframe sized memory may not port to Fuzix. Floating point
hardware is also not usually available so heavy floating point using
software will not run at any speed. In some cases software can be ported by
rewriting it not to use stupidly bad algorithms, in other cases the resource
constraint may be real.


