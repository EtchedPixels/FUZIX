.SUFFIXES: .c .rel

CC = fcc
CFLAGS = -DTINY
COPT = -O2

OBJS = initials.rel data.rel error.rel expr.rel function.rel gen.rel io.rel lex.rel main.rel preproc.rel \
       primary.rel stmt.rel sym.rel while.rel struct.rel

OBJ_Z80 = codez80.rel
OBJ_8080 = code8080.rel

INC = data.h defs.h prototype.h

OBJS_ALL = $(OBJS) $(OBJ_Z80) $(OBJ_8080)
all:    scc8080 sccz80


sccz80: $(OBJS) $(OBJ_Z80)
	$(CC) -o sccz80 --nostdio $(OBJS) $(OBJ_Z80)

scc8080: $(OBJS) $(OBJ_8080)
	$(CC) -o scc8080 --nostdio $(OBJS) $(OBJ_8080)

#scc6809: $(OBJS)
#	$(CC) -o scc8080 --nostdio $(OBJS)

clean:
	rm -f $(OBJ) scc8080 *.rel *~

.c.rel:
	$(CC) $(COPT) $(CFLAGS) -c $< -o $@

$(OBJS_ALL) : $(INC)
