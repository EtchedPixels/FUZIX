# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman

tools/syscall: tools/syscall.c ../Kernel/include/syscall_name.h

tools/binman: tools/binman.c
