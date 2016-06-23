.SUFFIXES:      .o .obj .c .asm

.c.o:
        ../src/sccvax $*.c
        as -o $*.o $*.s
        rm $*.s
ASSEMS =\
        abs.asm        atoi.asm       binary.asm\
        charclass.asm  fgets.asm      fputs.asm\
        getchar.asm    gets.asm       index.asm\
        itoa.asm       printn.asm     putchar.asm\
        puts.asm       reverse.asm    shell.asm\
        strcat.asm     strcmp.asm     strcpy.asm\
        strlen.asm     rand.asm \
        strncat.asm strncmp.asm strncpy.asm

OBJ =\
        abs.o        atoi.o       binary.o\
        charclass.o  fgets.o      fputs.o\
        getchar.o    gets.o       index.o\
        itoa.o       printn.o     putchar.o\
        puts.o       reverse.o    shell.o\
        strcat.o     strcmp.o     strcpy.o\
        strlen.o     rand.o \
        strncat.o strncmp.o strncpy.o
.c.asm:
        ../src/scc8080 $*.c
        mv $*.s $*.asm

all:    $(ASSEMS)

vaxlibc.a:      $(OBJ)
        ar ur vaxlibc.a  $(OBJ)
        ranlib vaxlibc.a
