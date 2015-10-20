CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc
FCC = ../../Library/tools/fcc -O2
PLATFORM =
#PLATFORM = -tzx128

PROGLOAD=`(cat ../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`

# Used for programs that make sdcc go boom or take hours
CC_CRAP = -mz80 --std-c99 -c --opt-code-size --max-allocs-per-node 1000 -I../../Library/include
ASM_OPT = -l -o -s
LINKER_OPT = -mz80 --nostdlib --no-std-crt0 --code-loc $(PROGLOAD) --data-loc  0
BINMAN = ../../Library/tools/binman

.SUFFIXES: .c .rel

SRCSNS = \
	qrun.c

SRCS  = 

SRCSBAD = 

OBJS = $(SRCS:.c=.rel)
OBJSNS = $(SRCSNS:.c=.rel)
OBJSBAD = $(SRCSBAD:.c=.rel)

LIBS = ../../Library/libs/syslib.lib

APPSNS = $(OBJSNS:.rel=)

APPS = $(OBJS:.rel=) $(OBJSBAD:.rel=) $(OBJSNS:.rel=)

all: $(APPS) sizes

$(OBJSBAD): $(SRCSBAD)
	$(CC) $(CC_CRAP) $(@:.rel=.c)

$(APPSNS): OPTS = --nostdio

$(OBJS): %.rel: %.c

$(OBJSNS): %.rel: %.c

$(OBJSBAD): %.rel: %.c

.c.rel:
	$(FCC) $(PLATFORM) -c $<

%: %.rel
	$(FCC) $(PLATFORM) $(OPTS) $< -o $@

sizes: $(APPS)
	ls -l $(APPS) >size.report

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin size.report

rmbak:
	rm -f *~ core

