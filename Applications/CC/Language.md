# Implementation

The following is a rough guide to how the compiler matches up with the C89
standard, and also some of the known problems or areas to be addresssed.


## Characters

All the required characters are implemented. No multibyte characters are
supported.

Trigraphs are not supported. This would be easy to add to cc0 but nobody
uses them.

## Translation Limits

External identifiers are significant to 14 characters

Many of the other limits are dependent upon memory and in an 8bit system
you will hit those limits first.

Pointer/array depth is limited to 7

Declarators are currently limited to a depth of 16. Given the pointer depth
restriction this is sufficient.

Functions may have up to 32 parameters although if these are not simple you
will hit other expression limits first.

Switches may have a maximum of 128 case labels including nested switches.

A struct may have up to 30 members

There can be a total of 50 enum constants.

Some parsing limits are not currently checked so ridiculous levels of struct
declaration or similar may run out of stack

Volatile and const are parsed but not enforced. The compiler does however
track the presence of volatile keywords within a statement and within
local variables and arguments. It acts accordingly for that expression or
all expressions in that function. Global volatile objects, or volatile
types for things like struct fields do not have this effect.

## Numerical Limits

All types must fit the bit pattern of the host unsigned long

Currently (to be fixed) a char is 8bits, a short is 16bit a long is 32bit.
Char defaults to unsigned.

Floating point is encoded in the cc0 front end in IEEE 32bit format. There
is currently no support for a 64bit double type but this would not be hard
to add subject to the bit pattern rule.

-32768 as a constant may be promoted to the wrong type size in some
situations. This is a bug to be fixed.

## Language

All the C89 keywords are recognized. The register directive is ignored,
volatile and const are parsed and mostly ignored. While const is useful it really
complicates type handling. Volatile on the other hand is broken by design.

Identifiers comply with the C standard.

Enumerations do not currently share their tag space with struct/union. In
addition enumerations are treated as their base type and no stricter
checking is done on assignment or values. In particular this means that
conversions between enum types will not cause a warning.

Multiple partial "compatible" function types are not supported (6.1.26). A
function is either fully defined or not defined. The ability to partially
define a C function in two scopes is rarely used in practice, silly and
would hugely complicate the compiler. Type equivalence between arrays with
dimensions and the same array with the first dimension undefined is
understood, as is pointer/array equivalence.

Character constant escapes also recognize '\e' for escape as is common in
many other compilers.

Bitfields are not supported.

## Operators

All the standard C operators are supported. Note that # and ## are managed
by the pre-processor not this compiler.

All the standard C punctuators are supported. The use of '#' is only
performed by the C pre-processor not this compiler.

The standard arithmetic conversion should be applying. This may need some
further debug.

Pointer scaling is supported, array scaling should also work correctly but
may need debug.

The behaviour of shifts exceeding the size of the type is target dependent.

## Declarations

The const and volatile qualifiers are parsed but mostly ignored.

The use of ellipsis C99 style as an indicator of partial array assignent is
supported.

