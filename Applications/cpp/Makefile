PLATFORM = 6809
CC = m6809-unknown-gcc
ASM = m6809-unknown-as
AR = m6809-unknown-ar
LINKER = m6809-unknown-ld
CFLAGS =  -I../../Library/include -I../../Library/include/6809 -Wall -pedantic -fno-strict-aliasing
COPT = -Os
LINKER_OPT = --oformat=raw -L../../Library/libs -lc6809
LIBGCCDIR = $(dir $(shell $(CC) -print-libgcc-file-name))
LINKER_OPT += -L$(LIBGCCDIR) -lgcc -Map=cpp.map
LINKER_OPT += --script=../util/$(TARGET).link
ASM_OPT = -o
CRT0 = ../../Library/libs/crt0_6809.o

OBJS = cpp.o hash.o main.o token1.o token2.o

all: cpp

cpp: main.o cpp.o hash.o token1.o token2.o
	$(LINKER) -o $@ $(LINKER_OPT) $(CRT0) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(COPT) -c $<

clean realclean:
	rm -f cpp main.o cpp.o hash.o token1.o token2.o tmp.h *~

maintclean: realclean
	rm -f token1.h token2.h

main.o: cc.h
cpp.o: cc.h
hash.o: cc.h
tree.o: cc.h

token1.o: token1.h
token2.o: token2.h

token1.h: token1.tok
	gperf -aptTc -N is_ctok -H hash1 token1.tok > tmp.h
	mv tmp.h token1.h

token2.h: token2.tok
	gperf -aptTc -k1,3 -N is_ckey -H hash2 token2.tok > tmp.h
	mv tmp.h token2.h
