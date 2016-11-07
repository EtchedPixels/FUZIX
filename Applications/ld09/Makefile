PLATFORM = 6809
CC = m6809-unknown-gcc
ASM = m6809-unknown-as
AR = m6809-unknown-ar
LINKER = m6809-unknown-ld
CFLAGS =  -I../../Library/include -I../../Library/include/6809 -Wall -pedantic -fno-strict-aliasing
COPT = -Os
LINKER_OPT = --oformat=raw -L../../Library/libs -lc6809
LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
LINKER_OPT += -L$(LIBGCCDIR) -lgcc -Map=ld09.map
LINKER_OPT += --script=../util/$(TARGET).link
ASM_OPT = -o
CRT0 = ../../Library/libs/crt0_6809.o

LIBDIR	=/usr/bin
LDFLAGS	=

DEFS	=

# An alternative file for a non-standard a.out.h (eg i386 linux on an Alpha)
#
# NATIVE=-DA_OUT_INCL='"a_out_local.h"' 

OBJS= dumps.o io.o ld.o readobj.o table.o typeconv.o linksyms.o writefuzix.o

all: ld09

ld09: $(OBJS)
	$(LINKER) -o $@ $(LINKER_OPT) $(CRT0) $(OBJS)

clean realclean clobber:
	rm -f *.o ld09 ld09r objchop catimage objdump09 *~

$(OBJS): align.h ar.h bindef.h byteord.h config.h const.h globvar.h obj.h \
	 syshead.h type.h

ar.h:
	test -f ar.h || \
	{ rm -f ar.h ; ln -s ../libc/include/ar.h . ; } || \
	ln ../libc/include/ar.h .

.c.o:
	$(CC) $(COPT) $(CFLAGS) $(DEFS) -c $< -o $@
