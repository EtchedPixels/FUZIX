.SUFFIXES: .c .rel

FCC = ../../Library/tools/fcc 
FCCOPTS = -O2
SRCS  = flashrom.c
OBJS = $(SRCS:.c=.rel)
APPS = $(OBJS:.rel=)

all: $(APPS)

$(OBJS): %.rel: %.c

.c.rel:
	$(FCC) $(FCCOPTS) -c $<

%: %.rel
	$(FCC) $(FCCOPTS) $< -o $@

clean:
	rm -f $(OBJS) $(APPS) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin size.report *~
