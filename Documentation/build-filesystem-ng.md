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

f 0644 /usr/share/liberror.txt            ../../Library/libs/liberror.txt

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
f 0644 /usr/man/man1/rpilot.1              doc/rpilot.1
d 0755 /usr/doc/rpilot
````

The definition of a package extends from the "package" keyword until the next
"package" keyword or until the end of the file. Thus, a file can contain
multiple package definitions.

The "package" keyword is followed by zero or more attributes and then by zero
or more commands.

There are currently 3 commands defined, each of them maps to one or more ucp
commands. The commands are parsed and an error-free-by-construction ucp script
is emitted and applied to the ucp executable:

* "d" creates and sets permissions on a directory. The directory must
be created leaf by leaf.

* "n" creates and sets properties on a node.

* "f" copies and sets permissions on a file. The destination
directory must exist. The source and destination file names are specified
independently. The source file is specified relative to the location of the
package file (so fuzix-basefs.pkg needs to ../ its way to liberror.txt but
fuzix-util.pkg can reference banner in the cwd).

Two attributes are shown:

* if-file - this is used in optional packages. It checks for the existence of the
named file. If the named file is not found, the package is disabled. This
provides a simple method to omit groups of files from applications that have not
been build.

* if-cpu - this is used to disable a package if its content is only applicable to
a particular target.


## Enabling and disabling packages

Every package is enabled by default. The "if-" attributes invoke rules that may
result in a package being disabled.

Once the attributes have been processed, each enabled package is expected to be
able to be processed to completions without errors.


## Current Status

The description reflects the current state of build-filesystem-ng; a complete
set of ·pkg files is in place, and it can successfully process them to generate
a working disk image.

The if-cpu attribute is recognised but ignored. The if-file attribute is
recognised and honoured. Command-line parameters can be used to get help and to
set the output filename, endian-ness and geometry.


## Futures

1. package name on command-line (see description of meta-package below)

2. additional attributes

 * require-pkg: attribute to allow dependencies to be declared. A simple iterative
solver will enable/disable packages and put them in the right order. Currently,
all packages have an implicit dependency on fuzix-basefs.pkg and it is "luck"
that it gets processed first.

 * set-cpu: set the attribute that is tested by if-cpu

 * (if-platform:, set-platform:)

 * disable-pkg: explicitly disable a package
 * enable-pkg: explicitly enable a package

  Example usage is to create a "meta-package" for example: Applications/fuzix-mysys.pkg

````
# My custom setup
package  mysys
disable-pkg  mysys

set-cpu  6809
require-pkg  basefs
disable-pkg  rpilot games
````


This package is disabled by default - so that it is not found and applied by a
'default' invocation of build-filesystem-ng. The package is explicitly enabled
at the command-line like this:

./build-filesystem-ng -p mysys

Package files are located in the normal way, but package "mysys" is required to
exist and is enabled (overriding the disable within its package file). Once
enabled, its other attributes come into play, guiding the package set that in
turn determines the content of the final disk image.

3. Add Kernel/platform to search list

and some more attributes, then a package can be found there like this (Kernel/platform/fuzix.pkg):

````
# This is a platform-specific file found through the Kernel/platform link
package  multicomp09
set-cpu  6809
set-platform  multicomp09
set-endian  big
set-geometry  256   65535
````


allowing build-filesystem to be run with no command-line arguments.

4. Add soft-link command


## Implementation

build-filesystem-ng is a PERL script which runs on the host system and invokes
the FUZIX mkfs, ucp and fsk executables.


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
