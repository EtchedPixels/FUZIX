# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman tools/fcc

clean:
	rm -f tools/syscall tools/binman tools/fcc

tools/syscall: tools/syscall.c ../Kernel/include/syscall_name.h
tools/syscall-z88dk: tools/syscall-z88dk.c ../Kernel/include/syscall_name.h

tools/binman: tools/binman.c
