CC = sdcc
ASM = sdasz80
AR = sdar
LINKER = sdcc
FCC = ../../../Library/tools/fcc
FCCOPTS = -O2
PLATFORM =
#PLATFORM = -tzx128

PROGLOAD=`(cat ../../../Kernel/platform/config.h; echo PROGLOAD) | cpp -E | tail -n1`

.SUFFIXES: .c .rel

SRCS  = ac.c accton.c at.c atrun.c col.c comm.c cron.c crypt.c dc.c dd.c \
	deroff.c diff3.c diff.c diffh.c join.c look.c makekey.c mesg.c \
	newgrp.c pr.c ptx.c rev.c split.c su.c sum.c test.c time.c tsort.c \
	wall.c

SRCSNS = ed.c

SRCSBAD =

OBJS = $(SRCS:.c=.rel)
OBJSNS = $(SRCSNS:.c=.rel)
OBJSBAD = $(SRCSBAD:.c=.rel)

LIBS = ../../../Library/libs/syslib.lib

APPSNS = $(OBJSNS:.rel=)

APPS = $(OBJS:.rel=) $(OBJSBAD:.rel=) $(OBJSNS:.rel=)

all: $(APPS) sizes


$(APPSNS): OPTS = --nostdio

$(OBJS): %.rel: %.c

$(OBJSNS): %.rel: %.c

$(OBJSBAD): %.rel: %.c
	$(FCC) $(PLATFORM) -c $<

.c.rel:
	$(FCC) $(PLATFORM) $(FCCOPTS) -c $<

%: %.rel
	$(FCC) $(PLATFORM) $(OPTS) $< -o $@

sizes: $(APPS)
	ls -l $(APPS) >size.report

clean:
	rm -f $(OBJS) $(APPS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin size.report

rmbak:
	rm -f *~ core

