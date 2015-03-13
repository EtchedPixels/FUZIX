FuzixOS: the build system


Building
========

To build a particular platform:

      make -j PLATFORM=msx2

This will build the kernel, apps, libc and filesystem image, leaving
the result as the obviously-named files in the current directory. The
kernel filenames may vary due to the platform, because they each have
different requirements.

Each platform gets its own set of object files, so you intermediate
files from different platforms won't step on each other. Full dependency
analysis is done. It shouldn't be necessary to do a clean, but 'make clean'
does the obvious thing.

To change the filesystem image or kernel locations, override FILESYSTEM
and/or KERNEL:

      make -j PLATFORM=msx2 FILESYSTEM=/tmp/fs.img KERNEL=/tmp/krnl.com


Building executables
====================

To build a user-space binary:

      $(OBJ)/Applications/emacs: $(OBJ)/Applications/emacs1.$O \
	  		$(OBJ)/Applications/emacs2.$O

The build system will figure the rest out for itself; it understands .c, .s and
.S source files, in the corresponding source directory; e.g.
Applications/emacs1.c and Applications/emacs2.c. Use $O as the object file
suffix, because some compilers (such as SDCC) don't use .o. To specify custom
variables, use this:

      $(OBJ)/Applications/emacs1.$O: CFLAGS += -DUSE_LESS_MEMORY

If you want the binary placed in the target filesystem, edit the script in
Standalone/filesystem-src/ucp-script.txt. The build system will notice this and
build your binary at the right time.

To build a host binary, do exactly the same thing, but use $(HOSTOBJ) instead of
$(OBJ).


Adding a platform
=================

Find an existing platform makefile in Build/platforms and clone it,
as Build/platforms/$NEWPLATFORM.mk. Include host-rules.mk and the
rules.mk for your compiler.

You should now be able to build the libc and filesystem image:

      make -j PLATFORM=yourplatform filesystem-yourplatform.bin

To build a kernel, clone an existing makefile from Kernel/platform-*/build.mk
into Kernel/platform-$NEWPLATFORM/build.mk; then do your kernel port.
CPU-specific but platform-independent kernel rules go in
Kernel/cpu-whatever/build.mk. These usually set up the kernel include paths and
set the segment names.

Once this is done, the kernel should build.


Adding a compiler
=================

Settings for the compiler 'thing' go in Build/thing-rules.mk. This contains
*all* the basic rules needed to build object files and link together target
executables. The basic required ruleset is:

    $(OBJ)/%.$O: $(TOP)/%.S |paths
	$(OBJ)/%.$O: $(OBJ)/%.S |paths
	$(OBJ)/%.$O: $(TOP)/%.c |paths
	$(OBJ)/%.$O: $(OBJ)/%.c |paths
	$(OBJ)/%.$O: $(TOP)/%.s |paths
	$(OBJ)/%.$A:
	$(OBJ)/Applications/%: $(CRT0) $(LIBC)

It also needs to define $(CRT0) and $(LIBC). Usually these won't need specific
rules (as they're just generic object and libraries, which are built using the
implicit rules).

The trailing |paths is important! This ensures that the paths target is invoked
before any source file is built; this is used to do some setup.

