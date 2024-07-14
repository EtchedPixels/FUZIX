ROOT=../../../..
FUZIX_ROOT=$(ROOT)
USERCPU=armm0
VERBOSE=1
include $(FUZIX_ROOT)/Target/rules.$(USERCPU)

UTILS = picoctl.c picogpio.c
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

%.o: %.c
	$(CC) $(CFLAGS) $(CCOPTS) -c $<
