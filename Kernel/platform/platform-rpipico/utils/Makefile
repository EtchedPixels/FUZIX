ROOT=../../../..
FUZIX_ROOT=$(ROOT)
USERCPU=armm0
VERBOSE=1
include $(FUZIX_ROOT)/Target/rules.$(USERCPU)


UTILS = ps.c
UTILSO = $(UTILS:.c=.o)
UTILSBIN = $(UTILS:.c=)

.PHONY: all clean

all: $(UTILSBIN)

clean:
	rm -f *.o
	rm -f $(UTILSBIN)
	rm -f progbase.h
	rm -f ps.c

$(UTILSBIN): %: %.o
	$(LINKER) $(CRT0) $^ -o $@ $(LINKER_OPT) $(LINKER_TAIL)

ps.c:
	cp ../../../../Applications/util/ps.c .
	patch ps.c < ps-rpipico.patch

%.o: %.c progbase.h
	$(CC) $(CFLAGS) $(CCOPTS) -c $<

progbase.h:
	nm ../build/fuzix.elf | grep progbase | awk '{print "#define PROGBASE 0x" $$1}' > progbase.h

