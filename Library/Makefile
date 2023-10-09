# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman tools/fcc tools/syscall_6303 \
     tools/syscall_6502 tools/syscall_68000 tools/syscall_8080 \
     tools/syscall_6809 tools/syscall-scc6809 tools/syscall_armm4 \
     tools/ack2fuzix tools/binman85 \
     tools/liberror tools/relocbin tools/relocbin65 tools/fuzixlib \
     tools/relocbinhc11 tools/relocbin6303 tools/relocbin8080 tools/shlibgen \
     tools/relocbin6809 tools/ns32ktoaout tools/syscall_riscv32 \
     tools/riscv32toaout tools/syscall_8086

clean:
	rm -f tools/syscall tools/binman tools/fcc tools/fsize tools/liberror
	rm -f tools/binman85 tools/relocbin6303 tools/relocbin6809 tools/shlibgen
	rm -f tools/relocbin tools/ack2fuzix tools/fuzixlib
	rm -f tools/syscall-z88dk tools/syscall_6502 tools/syscall_6809 tools/syscall_68000 tools/syscall-scc6809
	rm -f tools/syscall_pdp11 tools/syscall_8080 tools/syscall_6303 tools/syscall_esp8266 tools/syscall_8086
	rm -f tools/relocbin65 tools/relocbinhc11 tools/relocbin8080 tools/syscall_armm4
	rm -f tools/ns32ktoaout tools/syscall_riscv32 tools/riscv32toaout

tools/syscall: tools/syscall.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6303: tools/syscall_6303.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6502: tools/syscall_6502.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_8080: tools/syscall_8080.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6800: tools/syscall_6800.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_68000: tools/syscall_68000.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_6809: tools/syscall_6809.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall-scc6809: tools/syscall-scc6809.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_68hc11: tools/syscall_68hc11.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_8086: tools/syscall_8086.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_lx106: tools/syscall_lx106.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_pdp11: tools/syscall_pdp11.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_armm4: tools/syscall_armm4.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_wrx6: tools/syscall_wrx6.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_riscv32: tools/syscall_riscv32.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/binman: tools/binman.c
	$(CC) $(CFLAGS) -o $@ $<

tools/binman85: tools/binman85.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin: tools/relocbin.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin6303: tools/relocbin6303.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin65: tools/relocbin65.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin6809: tools/relocbin6809.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbin8080: tools/relocbin8080.c
	$(CC) $(CFLAGS) -o $@ $<

tools/relocbinhc11: tools/relocbinhc11.c
	$(CC) $(CFLAGS) -o $@ $<

tools/ns32ktoaout: tools/ns32ktoaout.c
	$(CC) $(CFLAGS) -o $@ $<

tools/riscv32toaout: tools/riscv32toaout.c
	$(CC) $(CFLAGS) -o $@ $<

tools/liberror: tools/liberror.c
	$(CC) $(CFLAGS) -o $@ $<

tools/ack2fuzix: tools/ack2fuzix.c
	$(CC) $(CFLAGS) -o $@ $<

tools/fuzixlib: tools/fuzixlib.c
	$(CC) $(CFLAGS) -o $@ $<

tools/shlibgen: tools/shlibgen.c
	$(CC) $(CFLAGS) -o $@ $<

install:
ifeq ($(USERCPU),z80)
	mkdir -p /opt/fcc/bin /opt/fcc/lib /opt/fcc/include
	mkdir -p /opt/fcc/include/6502 /opt/fcc/include/arpa
	mkdir -p /opt/fcc/include/msp430x
	mkdir -p /opt/fcc/include/net /opt/fcc/include/netinet
	mkdir -p /opt/fcc/include/readline
	mkdir -p /opt/fcc/include/sys
	install -m 0755 tools/binman tools/relocbin tools/fcc /opt/fcc/bin
	install -m 0644 include/*.h /opt/fcc/include
	install -m 0644 include/6502/*.h /opt/fcc/include/6502
	install -m 0644 include/arpa/*.h /opt/fcc/include/arpa
	install -m 0644 include/msp430x/*.h /opt/fcc/include/msp430x
	install -m 0644 include/net/*.h /opt/fcc/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/include/readline
	install -m 0644 include/sys/*.h /opt/fcc/include/sys
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
endif
ifeq ($(USERCPU),ez80_z80)
	mkdir -p /opt/fcc/bin /opt/fcc/lib /opt/fcc/include
	mkdir -p /opt/fcc/include/6502 /opt/fcc/include/arpa
	mkdir -p /opt/fcc/include/msp430x
	mkdir -p /opt/fcc/include/net /opt/fcc/include/netinet
	mkdir -p /opt/fcc/include/readline
	mkdir -p /opt/fcc/include/sys
	install -m 0755 tools/binman tools/relocbin tools/fcc /opt/fcc/bin
	install -m 0644 include/*.h /opt/fcc/include
	install -m 0644 include/6502/*.h /opt/fcc/include/6502
	install -m 0644 include/arpa/*.h /opt/fcc/include/arpa
	install -m 0644 include/msp430x/*.h /opt/fcc/include/msp430x
	install -m 0644 include/net/*.h /opt/fcc/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/include/readline
	install -m 0644 include/sys/*.h /opt/fcc/include/sys
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
endif
ifeq ($(USERCPU),6502)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),65c816)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),tms9995)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
endif
ifeq ($(USERCPU),6303)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),ns32k)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
endif
ifeq ($(USERCPU),wrx6)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),armm4)
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
