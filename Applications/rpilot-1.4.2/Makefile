CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc
FCC = ../../Library/tools/fcc -O2
PLATFORM =
#PLATFORM = -tzx128

.SUFFIXES: .c .rel

SRCS  = bind.c calc.c cmds.c condex.c debug.c err.c interact.c label.c \
	line.c main.c math.c rpilot.c rpinfo.c rstring.c stack.c var.c

INCS  = bind.h calc.h cmds.h condex.h debug.h err.h interact.h label.h \
	line.h main.h math.h rpilot.h rpinfo.h rstring.h stack.h var.h


OBJS = $(SRCS:.c=.rel)

LIBS = ../../Library/libs/syslib.lib

all: rpilot

rpilot: $(OBJS)
	$(FCC) $(PLATFORM) $(OBJS) -o $@

$(OBJS): $(INCS)

.c.rel:
	$(FCC) $(PLATFORM) -DNO_INTER -DIGNORE_HASH -c $<

%: %.rel
	$(FCC) $(PLATFORM) $< -o $@

clean:
	rm -f $(OBJS) rpilot $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin

rmbak:
	rm -f *~ core

