# Top level makefile for library

CFLAGS += -I../Kernel/include

all: bin/syscall bin/binman bin/fcc

clean:
	rm -f bin/syscall bin/binman bin/fcc

bin/syscall: bin/syscall.c ../Kernel/include/syscall_name.h
bin/syscall-z88dk: bin/syscall-z88dk.c ../Kernel/include/syscall_name.h

bin/binman: bin/binman.c
