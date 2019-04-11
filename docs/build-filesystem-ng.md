# Packaging/Disk Image Build System for FUZIX

## Overview

build-filesystem-ng is designed as a replacement for build-filesystem. If
accepted, it will be renamed and the old files retired.

build-filesystem-ng is a script for creating a FUZIX disk image. It has some of
the attributes of a package manager. The original motivation for its development
was to move away from the monolithic build-filesystem script. Using
build-filesystem-ng, each application or group of files can be described by a
separate package file which can be created and maintained in a distributed way.

build-filesystem-ng makes it easy to identify target-specific and
platform-specific files/file-sets in order to create a tailored disk image.


## Operation

When build-filesystem-ng is executed, it performs these steps in sequence:

* Search the Standalone and Application trees for package files. Package files
  are recognised by name. It's OK to have multiple package files with the same
  name.
* Determine whether each package is enabled or disabled.
* Re-order the package list, if necessary, to resolve declared dependencies of
  enabled packages.
* Generate a FUZIX disk image containing all of the elements (files,
  directories, nodes etc.) specified in the enabled packages


## Package Files

A package file is plain ASCII, typically hand-generated.

Each package file contains zero or more package definitions. Each package
definition is followed by zero or more attributes and then by zero or more
commands. A package definition is simply the name of the package. Package names
must be unique.


## Example

Here are 3 (cut-down) package files.

Standalone/filesystem-src/fuzix-basefs.pkg
````
package  basefs

d 0755 /
d 0755 /usr
d 0755 /dev
d 0755 /usr/share

f 0644 /usr/lib/liberror.txt            ../../Library/libs/liberror.txt

n 20666 512 /dev/tty   
````

Applications/util/fuzix-util.pkg
````
package  util

f 0755 /bin/banner      banner
f 0755 /bin/basename    basename
f 0755 /bin/man         man
f 0644 /usr/man/man1/man.1    man.1

package  z80-util
if-cpu  z80

f 0755 /bin/patchcpm    patchcpm

````

Applications/rpilot-1.4.2/fuzix-rpilot.pkg
````
package  rpilot
if-file  rpilot

f 0755 /usr/bin/rpilot                     rpilot
l      /usr/bin/rpilot                     /usr/bin/pilot
f 0644 /usr/man/man1/rpilot.1              doc/rpilot.1
d 0755 /usr/doc/rpilot

````

The format is line-based and uses \# as the comment-to-end-of-line character.

The definition of a package extends from the "package" keyword until the next
"package" keyword or until the end of the file. Thus, a file can contain
multiple package definitions.

The "package" keyword is followed by zero or more lines of attributes and then
by zero or more lines of commands. Within a line, keywords/attributes/commands
are space-separated from their arguments.

The following commands are (currently) defined:

* d - create and set permissions on a directory. The directory must be created
  leaf by leaf.
* n - create and set properties on a node.
* f - copy and set permissions on a file. The destination directory must
  exist. The source and destination file names are specified independently. The
  source file is specified relative to the location of the package file (so
  fuzix-basefs.pkg needs to ../ its way to liberror.txt but fuzix-util.pkg can
  reference banner in the cwd).
* l - link from existing file to new file. The full path of both files must
  be specified. The destination directory must exist.
* r - remove an existing file. The full path of the file must be specified.
* (a rename can be synthesised by a l followed by a r)

The following attributes are shown in the examples:

* if-file - this is used in optional packages. It checks for the existence of the
  named file. If the named file is not found, the package is disabled. This
  provides a simple method to omit groups of files from applications that have not
  been built.
* if-cpu - this is used to disable a package if its content is only applicable to
  a particular target.


## Enabling and disabling packages

Consider the following use-cases:

1. Build a disk image containing everything that's available
2. Build a disk image containing everything that is suitable for a specific target processor
3. Build a disk image containing a specific package or set of packages
4. Build a disk image omitting a specific package or set of packages

The first use-case is trivial; it is the default behaviour of the script. A
well-written package file will include the "if-file" attribute and so all
packages that have been built will be included in the disk image.

The remaining three use-cases require the use of a "meta-package". A
meta-package is disabled by default (and therefore will not form part of
"everything that's available" in the previous example). One single meta-package
can be enabled at the command-line.

The second use-case can be achieved using a package file like this:

````
# My custom setup
package  mysys09
disable-pkg  mysys09

set-cpu  6809
````

Which is applied like this:

````
./build-filesystem-ng -x -f fuzixfs.dsk -p mysys09
````

The attribute disable-pkg is used to disable the package by default. The
package is enabled by the -p command-line argument.

The attribute set-cpu provides a value that is tested by if-cpu (fuzix-util.pkg
above provides an example). The if-cpu test will fail (package will be disabled)
if a set-cpu attribute is found in an enabled package _and_ the arguments to
if-cpu and set-cpu mis-match (therefore, in the absence of a set-cpu attribute,
the if-cpu test will always pass; package will remain enabled).

There is also an attribute pair if-platform/set-platform. They work in the same
way as if-cpu/set-cpu.

Notes:

* The order in which package files are found/processed does not affect the
  decision-making
* The implementation is intended to cope will real use-cases. You can surely
  compose pathological cases that are mutually contradictory.

The last two use-cases take opposite approaches to the same problem. One says
"start with nothing and add a specific set of package", the other says "start
with everything and remove a specific set of packages". Here is a package file
for the "start with nothing" approach:

````
# A specific set of packages for my system
package  mysys-nothing
disable-pkg  mysys-nothing ALL
enable-pkg basefs games V7-games adventure
````

As before, it is selected using the -p command-line parameter.

This example introduces:

* The special/reserved package name ALL which is used to disable every package
* The idea that disable-pkg actually accepts a _list_ of packages
* The attribute enable-pkg which is used to specify a list of packages to enable


Notes:

* You can only use ALL with disable-pkg
* You should only used ALL once, in the package specified at the command-line
* The _order_ of the enable-pkg, disable-pkg attributes has no effect on the
  behaviour of ALL; it has the effect of disabling every package (except the one
  that contains it) before any enable-pkg attributes are considered.

Finally, here is a package file for the "start with everything" approach:

````
# An educational system. No games (boo!)
package  mysys-all
disable-pkg  mysys-all games V7-games adventure
````
As before, it is selected using the -p command-line parameter.


Each package referred to on the enable-pkg/disable-pkg list could itself be a
meta-package. The design of the meta-package depends upon whether you are using
the "start with everything" or the "start with nothing" approach. For example,
consider these two package definitions (conveniently stored in a single file):

````
# All the games
package  allgames
disable-pkg allgames
enable-pkg  games V7-games adventure

# None of the games
package no-allgames
disable-pkg no-allgames games V7-games adventure
````

And then the earlier examples would become:

````
# A specific set of packages for my system
package  mysys
disable-pkg  mysys ALL
enable-pkg basefs allgames
````
and:

````
# An educational system. No games (boo!)
package  mysys
disable-pkg  mysys
enable-pkg no-allgames    # enable it so it can disable packages!
````

Package files can be "nested" to arbitrary depth in this way.

Notes:

* The idea of including a "positive" and "negative" meta-package in a single
  file is a convenient way of leaving your options open
* The "no-" prefix to the package name has no significance to build-filesystem-ng
  but might be considered a useful working-practise.

Implementation detail: package enabling/disabling is a 6-stage process:

* Apply defaults: enable any package unless it is self-disabled
* Apply -p: enable a package referenced on the command-line
* Apply ALL if found: disable all packages except its containing package
* For all enabled packages, process any enable-pkg statements. Iterate until stable
* For all enabled packages, process any disable-pkg statements except self-disable (no iteration is required here)
* For all enabled packages, process any if-file if-cpu if-platform attributes (may result in packages being disabled)

Once the attributes have been processed, each enabled package is expected to be
able to be processed to completion without errors. Errors are fatal.


## Building Small Disk Images

Here are some techniques for building small disk images.

* Create a meta-package, use ALL to disable everything
* Refactor existing packages (within a single package file) into a minimal
  and a full-blown file set (see fuzix-util.pkg for an example of this) and
  then pull in the minimal parts
* Use r (remove) in the meta-package file to tactically trim away files you
  can do without


See Standalone/filesystem-src/fuzix-mini.pkg for an example.

## Building Custom Disk Images

Here are some techniques for building custom disk images.

* Create a meta-package
* Use the techniques elsewhere to customise the package list, if required
* There is no command to _rename_ a file, but you can synthesise a rename by
  a link followed by remove of the original file.
* You cannot overwrite a file (for example, to replace /etc/inittab with
  a version customised for your system) but you can synthesise an overwrite
  by a remove followed by a copy of the new file

Here is an example of these techniues:

````
# Custom package for my system
package deluxe
disable-pkg deluxe
# synthesise rename of /etc/init
l /etc/inittab /etc/inittab.orig
r /etc/inittab

# replace it with a version that starts up 2 terminals
f 0644 /etc/inittab   my_inittab
````

One 'gotcha' with this technique is that there is (currently) no control over
the order in which package files are processed, except that those in Standalone/
are processed before those in Applications/. See require-pkg in the 'futures'
section below.


## Futures

This section is a list of anticipated but not-yet-implemented changes.
Mail me with other requirements.

1. Dependencies, using new attribute require-pkg. -- A simple iterative solver
will put them in the right order. Currently, all packages have an implicit
dependency on fuzix-basefs.pkg and it is "luck" that it gets processed first.

2. Add Kernel/platform to search list

and some more attributes, then a package can be found there like this (Kernel/platform/fuzix.pkg):

````
# This is a platform-specific file found through the Kernel/platform link
package  multicomp09
set-cpu  6809
set-platform  multicomp09
set-endian  big
set-geometry  256   65535
````

3. allow enable-pkg disable-pkg to be repeated within a package file:
concatenate their argument lists. Currently ignore all but the last instance in
any packaged file.


## Implementation

build-filesystem-ng is a PERL script which runs on the host system and invokes
the FUZIX mkfs, ucp and fsk executables.

Each of the (currently-defined) commands maps to one or more ucp commands. A
correct-by-construction ucp script is generated (ucp-temp.txt) and applied to
the ucp executable.

The r(emove) command does not remove an entry from the ucp-temp.txt - it generates
a new (rm) entry in ucp-temp.txt.


## Restrictions and error-handling

Within a package file, # is treated as a comment character, with no escape. Text
from '#' to the end of the line is discarded.

Whitespace acts as a separator. Excess whitespace is ignored and discarded.

There is no accommodation for file-names that include whitespace or quotes or
slashes or anything weird.

Attempts to multiply-define a directory/node/file generate an error, referenced
back to the source directory/file/line (ucp allows a file to be redefined but I
do not).

build-filesystem-ng processes all package files to completion, reporting all
errors. If this process is successful it goes on to apply the ucp script. An
error in the execution of the ucp script represents a bug in
build-filesystem-ng.
