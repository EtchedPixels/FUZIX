#
#	IntCode engine
#

FCC = fcc
FCCOPTS = -O2 #--nostdio
PLATFORM =

SRC = icint.c blib.c
OBJ = $(SRC:.c=.rel)

.SUFFIXES: .c .rel

all: icint

.c.rel:
	$(FCC) $(PLATFORM) $(FCCOPTS) -c $<

icint: $(OBJ)
	$(FCC) $(PLATFORM) $(FCCOPTS) -o $@ $(OBJ)

clean:
	rm -f icint *.rel *.asm *.lst core *.sym *.map *.noi *.lk *.ihx *.tmp *.bin *~
