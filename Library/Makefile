# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman tools/fcc tools/syscall_6502 \
     tools/syscall_6809 tools/binman tools/fcc tools/liberror

clean:
	rm -f tools/syscall tools/binman tools/fcc tools/fsize tools/liberror
	rm -f tools/syscall-z88dk tools/syscall_6502 tools/syscall_6809

tools/syscall: tools/syscall.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall-z88dk: tools/syscall-z88dk.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6502: tools/syscall_6502.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6809: tools/syscall_6809.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/binman: tools/binman.c
	$(CC) $(CFLAGS) -o $@ $<

tools/liberror: tools/liberror.c
	$(CC) $(CFLAGS) -o $@ $<
