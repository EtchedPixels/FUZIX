CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc
FCC = ../../Library/tools/fcc
FCCOPTS = -O2
PLATFORM =
#PLATFORM = -tzx128

PROGLOAD=`(cat ../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`

.SUFFIXES: .c .rel

SRCS  = picol.c

OBJS = $(SRCS:.c=.rel)

LIBS = ../../Library/libs/syslib.lib

APPS = $(OBJS:.rel=)

all: $(APPS) sizes

$(OBJS): %.rel: %.c

.c.rel:
	$(FCC) $(PLATFORM) $(FCCOPTS) -c $<

%: %.rel
	$(FCC) $(PLATFORM) $(OPTS) $< -o $@

sizes: $(APPS)
	ls -l $(APPS) >size.report

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin size.report *.o

rmbak:
	rm -f *~ core

