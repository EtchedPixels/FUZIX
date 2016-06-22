.SUFFIXES: .c .rel

CC = fcc
CFLAGS = -DTINY -DM8080
COPT = -O2

OBJS = initials.rel data.rel error.rel expr.rel function.rel gen.rel io.rel lex.rel main.rel preproc.rel \
       primary.rel stmt.rel sym.rel while.rel code6809.rel code8080.rel struct.rel

INC = data.h defs.h prototype.h

all:    scc8080


scc8080: $(OBJS)
	$(CC) -o scc8080 --nostdio $(OBJS)

#scc6809: $(OBJS)
#	$(CC) -o scc8080 --nostdio $(OBJS)

clean:
	rm -f $(OBJ) scc8080 *.rel *~

.c.rel:
	$(CC) $(COPT) $(CFLAGS) -c $< -o $@

$(OBJS) : $(INC)
