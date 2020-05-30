# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman tools/fcc tools/syscall_6303 \
     tools/syscall_6502 tools/syscall_68000 tools/syscall_8080 \
     tools/syscall_6809 tools/syscall-scc6809 \
     tools/ack2fuzix tools/binman \
     tools/liberror tools/relocbin tools/fuzixlib

clean:
	rm -f tools/syscall tools/binman tools/fcc tools/fsize tools/liberror
	rm -f tools/relocbin tools/ack2fuzix tools/fuzixlib
	rm -f tools/syscall-z88dk tools/syscall_6502 tools/syscall_6809 tools/syscall_68000 tools/syscall-scc6809
	rm -f tools/syscall_pdp11 tools/syscall_8080 tools/syscall_6303

tools/syscall: tools/syscall.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6303: tools/syscall_6303.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6502: tools/syscall_6502.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_8080: tools/syscall_8080.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_68000: tools/syscall_68000.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6809: tools/syscall_6809.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall-scc6809: tools/syscall-scc6809.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_pdp11: tools/syscall_pdp11.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/binman: tools/binman.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin: tools/relocbin.c
	$(CC) $(CFLAGS) -o $@ $<

tools/liberror: tools/liberror.c
	$(CC) $(CFLAGS) -o $@ $<

tools/ack2fuzix: tools/ack2fuzix.c
	$(CC) $(CFLAGS) -o $@ $<

tools/fuzixlib: tools/fuzixlib.c
	$(CC) $(CFLAGS) -o $@ $<

install:
ifeq ($(USERCPU),z80)
	mkdir -p /opt/fcc/bin /opt/fcc/lib /opt/fcc/include
	mkdir -p /opt/fcc/include/6502 /opt/fcc/include/arpa
	mkdir -p /opt/fcc/include/msp430x /opt/fcc/include/netinet
	mkdir -p /opt/fcc/include/readline
	mkdir -p /opt/fcc/include/sys
	install -m 0755 tools/binman tools/relocbin tools/fcc /opt/fcc/bin
	install -m 0644 include/*.h /opt/fcc/include
	install -m 0644 include/6502/*.h /opt/fcc/include/6502
	install -m 0644 include/arpa/*.h /opt/fcc/include/arpa
	install -m 0644 include/msp430x/*.h /opt/fcc/include/msp430x
	install -m 0644 include/netinet/*.h /opt/fcc/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/include/readline
	install -m 0644 include/sys/*.h /opt/fcc/include/sys
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/include/sys/
endif
ifeq ($(USERCPU),ez80_z80)
	mkdir -p /opt/fcc/bin /opt/fcc/lib /opt/fcc/include
	mkdir -p /opt/fcc/include/6502 /opt/fcc/include/arpa
	mkdir -p /opt/fcc/include/msp430x /opt/fcc/include/netinet
	mkdir -p /opt/fcc/include/readline
	mkdir -p /opt/fcc/include/sys
	install -m 0755 tools/binman tools/relocbin tools/fcc /opt/fcc/bin
	install -m 0644 include/*.h /opt/fcc/include
	install -m 0644 include/6502/*.h /opt/fcc/include/6502
	install -m 0644 include/arpa/*.h /opt/fcc/include/arpa
	install -m 0644 include/msp430x/*.h /opt/fcc/include/msp430x
	install -m 0644 include/netinet/*.h /opt/fcc/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/include/readline
	install -m 0644 include/sys/*.h /opt/fcc/include/sys
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/include/sys/
endif
ifeq ($(USERCPU),6502)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),6303)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
