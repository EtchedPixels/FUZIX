# Coding Style

:warning: This is a descriptive not a prescriptive document.

## Shared Code

Code that is part of the core tree used for everything should adhere to the
following rules

### Compiler Limits

#### No use of (void)foo() style casting
This is not supported by all the compilers we use.

#### No floating point
The floating point support varies by platform and even when present is often
very slow on a small micro, and adds a lot of code.

#### No long division in kernel code
On several platforms this sucks in large non re-entrant support routines tha
would require saving more state on a context switch

#### Less than 256 bytes within any given stack frame
cc65 can't handle large stack offsets, and on Z80, 8085 and 6800 series
processors there is much great expense accessing beyond the 128 or 256 byte
range around the stack frame.

#### All identifiers unique within 14 characters
This is the worst case for the compilers used.

#### No // comments
Not all compilers support them.

### Performance

#### Place initialized auto variables together when possible
Several compilers generate shorter and cleaner initialization code on
function entry if local variables are initialized in their declaration and
the initializers are together.

#### Try to keep one active pointer at a time
There are limited indexing registers on some targets so generating a series
of references to one pointer is far better than working with several. The
kernel tries to cluster things into arrays of structs with all the
properties for this reason.

#### Pre-calculate loop limit expressions when possible

````
	while (x < n + 2 * p) { func(); }
````

often requires the math is evaluated each time. On a modern processor nobody
notices. On an 8080 you do.

#### Use uint_fast8_t not uint8_t unless you are working with arrays
C has some annoying rules about type promotion and arithmetic which mean
that on certain systems uint8_t is actually expensive. On such systems
uint_fast8_t will be a faster unsigned type.

#### Use unsigned maths whenever possible
Sign extension is really expensive on some processors. The 8080 also lacks
signed comparisons making signed operations more expensive.

### Coding Style

Currently the code mostly uses a Linux like style but with a confusion of 4
and 8 character indenting. This should be moved to 8 character indenting for
core code.  Reformatting is being done gradually when appropriate.

### Custom Defines

The kernel uses a few extra type defines of its own.

#### \_\_packed

Tag an array as packed. On many systems this is the natural order of things
and they don't support any "packing" attribute. On gcc based targets it
expands to the expected attribute notation.

#### barrier()

A store/load barrier. On many of the ports this is actually a no-op because
the compiler isn't smart enough to move load/stores over a statement
boundary.

#### inline

Mark a function as inlineable if supported by the compiler

#### \_\_fastcall

On certain targets single argument functions have a special calling pattern,
especially with calls to assembly code. This is used to mark such functions.
Generally speaking the use of \_\_fastcall is only done for platform specific
uses.

#### staticfast

Marks a variable that can be turned into a static for performance on
processors that benefit from not having too many local variables to juggle
at once. This notably affects Z80 - use with care. Various general purpose
values used in syscalls are instead referenced via the udata for the same
effect but without re-entrancy problems.

#### regptr

Mark a variable that is used a lot (more than about 8 times) in a function
as a pointer. This is used to help the 6502 compiler as the other compilers
generally either do registerisation on their own or don't do it at all.

#### used(x)

Tell the compiler that x is actually used and silence any warnings that it
is not referenced.

## Target specific code

This should be self consistent but the constraints on the core code do not
apply. Things like compiler extensions can be freely used, along with other
types and features.

Target specific code that isn't maintained actively by a contributor is a
candidate for reformatting, particularly if the style is strange. Target
code that is actively changing is not.

## Imported Code

Generally imported code keeps the style it had originally unless that style
is sufficiently weird to be a problem (eg the Bourne Shell), or conflicts
wiht the constraints of the tool chain for a platform using it.

