# CFLAGS= -Wall

ar: append.c ar.c archive.c contents.c delete.c extract.c misc.c move.c \
	print.c replace.c strmode.c archive.h extern.h pathnames.h
	cc -o ar $(CFLAGS) append.c ar.c archive.c contents.c delete.c \
		extract.c misc.c move.c print.c replace.c strmode.c

clean:
	rm -f ar *.o
