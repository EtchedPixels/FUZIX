PLATFORM = 6809
CC = m6809-unknown-gcc
# These are wrappers for lwasm and lwar
ASM = m6809-unknown-as
AR = m6809-unknown-ar
LINKER = lwlink
CFLAGS =  -I../../Library/include -I../../Library/include/6502 -Wall -pedantic -fno-strict-aliasing
COPT = -Os
LINKER_OPT = --format=raw -L../../Library/libs -lc6809
LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
LINKER_OPT += -L$(LIBGCCDIR) -lgcc -m ld09.map
LINKER_OPT += --script=../util/$(TARGET).link
ASM_OPT = -o
CRT0 = ../../Library/libs/crt0_6809.o

LIBDIR	=/usr/bin
LDFLAGS	=

# Will need some of these if you want native executables on non-Linux/i386
# -DDETECTAOUT			# Turn on detection.
# -DV7_A_OUT			# a.out.h is like V7
# -DBSD_A_OUT			# a.out.h is like BSD
# -DSTANDARD_GNU_A_OUT		# a.out.h is like GNU normal.
#
# -DREL_OUTPUT -DBUGCOMPAT	# -r Produces weird *.o files.
#
DEFS	=-DREL_OUTPUT

# An alternative file for a non-standard a.out.h (eg i386 linux on an Alpha)
#
# NATIVE=-DA_OUT_INCL='"a_out_local.h"' 

OBJS= dumps.o io.o ld.o readobj.o table.o typeconv.o linksyms.o mkar.o \
      writebin.o writeemu.o

all: ld09 objchop

ld09: $(OBJS)
	$(LINKER) -o $@ $(LINKER_OPT) $(CRT0) $(OBJS)

objchop: objchop.o
	$(LINKER) -o $@ $(LINKER_OPT) $(CRT0) $(OBJS)

clean realclean clobber:
	rm -f *.o ld09 ld09r objchop catimage objdump09 *~

$(OBJS): align.h ar.h bindef.h byteord.h config.h const.h globvar.h obj.h \
	 syshead.h type.h x86_aout.h

ar.h:
	test -f ar.h || \
	{ rm -f ar.h ; ln -s ../libc/include/ar.h . ; } || \
	ln ../libc/include/ar.h .

writebin.o: writebin.c
	$(CC) $(CFLAGS) $(DEFS) $(NATIVE) -c $<

writerel.o: writebin.c

.c.o:
	$(CC) $(COPT) $(CFLAGS) $(DEFS) -c $< -o $@
