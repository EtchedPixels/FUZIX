# Makefile for osh
#
# @(#)$Id: dd28962f0dfb54cc5a71b03b72174fc6fd24e77e $
#
# Begin CONFIGURATION
#
# See the INSTALL file for build and install instructions.
#

#
# Choose where and how to install the binaries and manual pages.
#
DESTDIR?=
PREFIX?=	/usr/local
BINDIR?=	$(PREFIX)/bin
LIBEXECDIR?=	$(PREFIX)/libexec/$(OSH_VERSION)
DOCDIR?=	$(PREFIX)/share/doc/$(OSH_VERSION)
EXPDIR?=	$(DOCDIR)/examples
MANDIR?=	$(PREFIX)/share/man/man1
SYSCONFDIR?=	$(PREFIX)/etc
#BINGRP=		-g bin
BINMODE=	-m 0555
#MANGRP=		-g bin
MANMODE=	-m 0444

#
# Build utilities (SHELL must be POSIX-compliant)
#
INSTALL?=	/usr/bin/install
SHELL=		/bin/sh

#
# Preprocessor, compiler, and linker flags
#
#	If the compiler gives errors about any of flags specified
#	by `OPTIONS' or `WARNINGS' below, comment the appropriate
#	line(s) with a `#' character to fix the compiler errors.
#	Then, try to build again by doing a `make clean ; make'.
#
#CPPFLAGS=
OPTIONS=	-std=c99 -pedantic
#OPTIONS+=	-fstack-protector
WARNINGS=	-Wall -W
# NOTE: It seems that -Wno-unused-result first appeared in gcc-4.4.0 .
#WARNINGS=	-Wno-unused-result
#WARNINGS+=	-Wstack-protector
#WARNINGS+=	-Wshorten-64-to-32
#CFLAGS+=	-g
CFLAGS+=	-Os $(OPTIONS) $(WARNINGS)
#LDFLAGS+=	-static

#
# End CONFIGURATION
#
# !!! ================= Developer stuff below... ================= !!!
#

#
# Adjust CFLAGS for OSXCFLAGS and LDFLAGS for OSXLDFLAGS if needed.
#
OSXCFLAGS?=
CFLAGS+=	$(OSXCFLAGS)
OSXLDFLAGS?=
LDFLAGS+=	$(OSXLDFLAGS)

#
# Include osh date and version from Makefile.config .
#
include	./Makefile.config

OSH=	osh
SH6=	sh6 glob
UBIN=	fd2 goto if
GHEAD=	config.h defs.h
ERRSRC=	err.h err.c
PEXSRC=	pexec.h pexec.c
SIGSRC=	sasignal.h sasignal.c
INTSRC=	strtoint.h strtoint.c
OBJ=	err.o fd2.o glob.o goto.o if.o osh.o pexec.o sasignal.o sh6.o strtoint.o util.o v.o
OSHMAN=	osh.1.out
SH6MAN=	sh6.1.out glob.1.out
UMAN=	fd2.1.out goto.1.out if.1.out
MANALL=	$(OSHMAN) $(SH6MAN) $(UMAN)
SEDSUB=	-e 's|@OSH_DATE@|$(OSH_DATE)|' \
	-e 's|@OSH_VERSION@|$(OSH_VERSION)|' \
	-e 's|@LIBEXECDIR@|$(LIBEXECDIR)|' \
	-e 's|@SYSCONFDIR@|$(SYSCONFDIR)|'

DEFS=	-DOSH_VERSION='"$(OSH_VERSION)"' -DLIBEXECDIR='"$(LIBEXECDIR)"' -DSYSCONFDIR='"$(SYSCONFDIR)"'

.SUFFIXES: .1 .1.out .c .o

.1.1.out:
	sed $(SEDSUB) <$< >$@

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEFS) $<

#
# Build targets
#
all: oshall sh6all

oshall: $(OSH) $(OSHMAN) $(UMAN)

sh6all: $(SH6) $(SH6MAN) utils

utils: $(UBIN) $(UMAN)

man: $(MANALL)

osh: sh.h v.c osh.c util.c $(GHEAD) $(ERRSRC) $(PEXSRC) $(SIGSRC) $(INTSRC)
	@$(MAKE) $@bin

sh6: sh.h v.c sh6.c $(GHEAD) $(ERRSRC) $(PEXSRC) $(SIGSRC)
	@$(MAKE) $@bin

glob: v.c glob.c $(GHEAD) $(ERRSRC) $(PEXSRC)
	@$(MAKE) $@bin

if: v.c if.c $(GHEAD) $(ERRSRC) $(PEXSRC) $(SIGSRC) $(INTSRC)
	@$(MAKE) $@bin

goto: v.c goto.c $(GHEAD) $(ERRSRC)
	@$(MAKE) $@bin

fd2: v.c fd2.c $(GHEAD) $(ERRSRC) $(PEXSRC)
	@$(MAKE) $@bin

$(OBJ)                                                        : $(GHEAD)
fd2.o glob.o goto.o if.o osh.o pexec.o sh6.o util.o strtoint.o: err.h
fd2.o glob.o if.o osh.o sh6.o util.o                          : pexec.h
if.o osh.o sh6.o                                              : sasignal.h
if.o osh.o util.o                                             : strtoint.h
osh.o sh6.o util.o                                            : sh.h
err.o                                                         : $(ERRSRC)
pexec.o                                                       : $(PEXSRC)
sasignal.o                                                    : $(SIGSRC)
strtoint.o                                                    : $(INTSRC)

config.h: Makefile Makefile.config mkconfig
	$(SHELL) ./mkconfig

oshbin: v.o osh.o err.o util.o pexec.o sasignal.o strtoint.o
	$(CC) $(LDFLAGS) -o osh v.o osh.o err.o util.o pexec.o sasignal.o strtoint.o $(LIBS)

sh6bin: v.o sh6.o err.o pexec.o sasignal.o
	$(CC) $(LDFLAGS) -o sh6 v.o sh6.o err.o pexec.o sasignal.o $(LIBS)

globbin: v.o glob.o err.o pexec.o
	$(CC) $(LDFLAGS) -o glob v.o glob.o err.o pexec.o $(LIBS)

ifbin: v.o if.o err.o pexec.o sasignal.o strtoint.o
	$(CC) $(LDFLAGS) -o if v.o if.o err.o pexec.o sasignal.o strtoint.o $(LIBS)

gotobin: v.o goto.o err.o
	$(CC) $(LDFLAGS) -o goto v.o goto.o err.o $(LIBS)

fd2bin: v.o fd2.o err.o pexec.o
	$(CC) $(LDFLAGS) -o fd2 v.o fd2.o err.o pexec.o $(LIBS)

#
# Test targets
#
check: all
	@( trap '' INT QUIT && cd tests && ../osh run.osh osh sh6 )

check-newlog: all
	@( trap '' INT QUIT && cd tests && ../osh run.osh -newlog osh sh6 )

#
# Install targets
#
DESTBINDIR=	$(DESTDIR)$(BINDIR)
DESTLIBEXECDIR=	$(DESTDIR)$(LIBEXECDIR)
DESTDOCDIR=	$(DESTDIR)$(DOCDIR)
DESTEXPDIR=	$(DESTDIR)$(EXPDIR)
DESTMANDIR=	$(DESTDIR)$(MANDIR)
install: install-oshall install-sh6all

install-oshall: oshall install-osh install-uman

install-sh6all: sh6all install-sh6 install-utils

install-utils: install-ubin install-uman

install-osh: $(OSH) $(OSHMAN) install-dest
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) osh        $(DESTBINDIR)
	$(INSTALL) -c    $(MANGRP) $(MANMODE) osh.1.out  $(DESTMANDIR)/osh.1

install-sh6: $(SH6) $(SH6MAN) install-dest install-destlibexec
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) sh6        $(DESTBINDIR)
	$(INSTALL) -c    $(MANGRP) $(MANMODE) sh6.1.out  $(DESTMANDIR)/sh6.1
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) glob       $(DESTLIBEXECDIR)
	$(INSTALL) -c    $(MANGRP) $(MANMODE) glob.1.out $(DESTMANDIR)/glob.1

install-ubin: $(UBIN) install-dest install-destlibexec
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) fd2        $(DESTLIBEXECDIR)
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) goto       $(DESTLIBEXECDIR)
	$(INSTALL) -c -s $(BINGRP) $(BINMODE) if         $(DESTLIBEXECDIR)

install-uman: $(UMAN) install-dest
	$(INSTALL) -c    $(MANGRP) $(MANMODE) fd2.1.out  $(DESTMANDIR)/fd2.1
	$(INSTALL) -c    $(MANGRP) $(MANMODE) goto.1.out $(DESTMANDIR)/goto.1
	$(INSTALL) -c    $(MANGRP) $(MANMODE) if.1.out   $(DESTMANDIR)/if.1

install-dest:
	test -d $(DESTBINDIR) || { umask 0022 && mkdir -p $(DESTBINDIR) ; }
	test -d $(DESTMANDIR) || { umask 0022 && mkdir -p $(DESTMANDIR) ; }

install-destlibexec:
	test -d $(DESTLIBEXECDIR) || { umask 0022 && mkdir -p $(DESTLIBEXECDIR) ; }
	$(INSTALL) -c $(MANGRP) $(MANMODE) README.libexec $(DESTLIBEXECDIR)/README

install-doc:
	test -d $(DESTDOCDIR) || { umask 0022 && mkdir -p    $(DESTDOCDIR) ; }
	$(INSTALL) -c    $(MANGRP) $(MANMODE) [ACDILNP]* R*E $(DESTDOCDIR)

install-exp:
	test -d $(DESTEXPDIR) || { umask 0022 && mkdir -p $(DESTEXPDIR) ; }
	$(INSTALL) -c    $(MANGRP) $(MANMODE) examples/*  $(DESTEXPDIR)

#
# Cleanup targets
#
clean-man:
	rm -f $(MANALL)
clean-obj:
	rm -f $(OBJ)
clean: clean-man clean-obj
	rm -f $(OSH) $(SH6) $(UBIN) config.h

#
# Release target
#
release: clean
	rm -rf .release
	oshrelease .release $(OSH_VERSION)
