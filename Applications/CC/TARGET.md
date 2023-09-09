# Retargetting the compiler

## Concepts

The compiler backend is fed nodes from parse trees plus some other callbacks
are made for branches and the like. Most nodes correspond to the C operation
but there are also nodes for constants, and loading or storing
global/static, local variables and arguments.

One or two things contain a certain amount of magic that isn't totally
clean. The backend turns ?:, && and || into branches. Function calls also
have a few quirks that are pushed into the backend and handled simply in
the example  (T_CLEANUP, T_COMMA, T_FUNCCALL in the backend-default.c).

The abstract model of the processor used is a single working register that
is big enough to hold any type, a stack for data, a call/return stack (which
may be the same stack) and local variables and arguments which are on the
stack.

## Minimal

The base compiler is designed to be generate bad code for any hardware. The
minimal changes you need to make to generate code for your processor
assuming 16bit integers, and no strange alignment rules are

* Copy backend-default.c
* Update the assembler directives to work with your assembler
* Update gen_jump/gen_jfalse/gen_jtrue to generate branches that work with your processor
* Update gen_helpcall to use the right instruction for your processor
* Update the entry/prologue for functions to use the right return instruction

And that's it. You'll get a long stream of helper calls. You can actually
change the helper call to generate bytecodes of some form along with the
branches and you'll get a bytecode machine instead.

## First steps

Each node is offered to the code generator in your backend before a helper
is generated. If the gen_node function returns a zero indicating it does not
know how to generate the node then the backend will generate a helper.

This allows you to replace helpers one by one with native
code where the native code is short enough.

Obvious candidates are

* T_CONSTANT - loads a constant
* T_LABEL - loads the address of a literal (currently a text string)
* T_NAME - load the address of a static/global variable
* T_LOCAL - load the address of a local variable
* T_ARGUMENT - load the address of a function argument

In addition the gen_push helper can be replaced with native code to stack
the working register - at least for easy sizes.

At the point the helper is called any left hand side argument has been
placed on the stack and any right hand side argument is in the working
register. The helpers (or native code) are expected to consume the value
of the correct size from the stack.

## Using gen_direct

The gen_direct helper is invoked before the left hand side has been pushed
and the right hand side loaded. This allows some simpler operations to be
handled more cleanly. For example if the right hand side is a constant then
it's often possible to directly add the value rather than going via the
stack. The degree this is possibly depends a lot on the processor. The 6809
code generator is able to generate a lot of constant, stack and variable
based operations this way, whilst the 8080 is far less able. This is
called before gen_node and if it returns 1 gen_node will not be invoked.

## Using gen_shortcut

Trees are offered to gen_direct so that the target can match subtrees it can
generate code patterns for directly. This is useful for cases that can be
simplified and for spotting common patterns and replacing them with cut and
paste optimal code. This is called before gen_direct and if it returns 1 the
entire tree from this node downwards is deemed processed.

## Using gen_unidirect

This serves the same purpose for a single parameter operation (such as
negate or dereference). It allows the generation of more efficient
code on some processors for things like ~name or negate of constant.

## Tree rewriting

The expression trees are offered node by node to the gen_rewrite_node()
function. The nodes are walked left to right bottom up so that rewrites
can propagate up the tree as they simplify things above.

There are various uses for rewrites from handling processor weirdnesses to
compacting things like dereferences of names and local variables or their
assignment to shortcutting things that can be done directly, like calling
a function by name rather than through a variable.

The existing examples mostly use it to rewrite object references they
can perform more efficiently into single nodes rather than being a
dereference of a value loaded into the work register first. This removes
a lot of pushing and popping for most processor types.
