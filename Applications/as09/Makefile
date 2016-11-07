PLATFORM = 6809
CC = m6809-unknown-gcc
ASM = m6809-unknown-as
AR = m6809-unknown-ar
LINKER = m6809-unknown-ld
CFLAGS =  -I../../Library/include -I../../Library/include/6809 -Wall -pedantic -fno-strict-aliasing
COPT = -Os
LINKER_OPT = --oformat=raw -L../../Library/libs -lc6809
LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
LINKER_OPT += -L$(LIBGCCDIR) -lgcc -Map=as09.map
LINKER_OPT += --script=../util/$(TARGET).link
ASM_OPT = -o
CRT0 = ../../Library/libs/crt0_6809.o

OBJS		=as.o assemble.o errors.o express.o \
		 genbin.o genlist.o genobj.o gensym.o \
		 keywords.o macro.o mops.o pops.o readsrc.o \
		 scan.o table.o typeconv.o alloc.o

all: as09

as09: $(OBJS)
	$(LINKER) -o $@ $(LINKER_OPT) $(CRT0) $(OBJS)

clean realclean clobber:
	rm -f *.o as09 *~

.c.o:
	$(CC) $(CFLAGS) $(COPT) -c $<

$(OBJS): const.h errors.h

as.o: const.h type.h byteord.h macro.h file.h flag.h globvar.h
assemble.o: const.h type.h address.h globvar.h opcode.h scan.h
error.o: const.h type.h
express.o: const.h type.h address.h globvar.h scan.h source.h
genbin.o: const.h type.h address.h file.h globvar.h
genlist.o: const.h type.h address.h flag.h file.h globvar.h macro.h scan.h \
	   source.h
genobj.o: const.h type.h address.h file.h globvar.h
gensym.o: const.h type.h flag.h file.h globvar.h
keywords.o: const.h type.h globvar.h opcode.h
macro.o: const.h type.h globvar.h scan.h macro.h
mops.o: const.h type.h globvar.h opcode.h scan.h address.h
pops.o: const.h type.h address.h flag.h globvar.h opcode.h scan.h
readsrc.o: const.h type.h flag.h file.h globvar.h macro.h scan.h source.h
scan.o: const.h type.h globvar.h scan.h 
table.o: const.h type.h globvar.h opcode.h scan.h
