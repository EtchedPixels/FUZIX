# Top level makefile for library

CFLAGS += -I../Kernel/include

all: tools/syscall tools/binman tools/syscall_6303 \
     tools/syscall_6502 tools/syscall_68000 tools/syscall_8080 \
     tools/syscall_6809 tools/syscall-scc6809 tools/syscall_armm4 \
     tools/binman85 \
     tools/liberror tools/relocbin tools/relocbin65 tools/fuzixlib \
     tools/relocbinhc11 tools/relocbin6303 tools/relocbin8080 tools/shlibgen \
     tools/relocbin6809 tools/ns32ktoaout tools/syscall_riscv32 \
     tools/riscv32toaout tools/syscall_8086

clean:
	rm -f tools/syscall tools/binman tools/fsize tools/liberror
	rm -f tools/binman85 tools/relocbin6303 tools/relocbin6809 tools/shlibgen
	rm -f tools/relocbin tools/ack2fuzix tools/fuzixlib
	rm -f tools/syscall_z80 tools/syscall_6502 tools/syscall_6809 tools/syscall_68000 tools/syscall-scc6809
	rm -f tools/syscall_pdp11 tools/syscall_8080 tools/syscall_6303 tools/syscall_esp8266 tools/syscall_8086
	rm -f tools/syscall_6800 tools/syscall_68hc11 tools/syscall_8085 tools/syscall_ns32k
	rm -f tools/syscall_super8 tools/syscall_tms9995 tools/syscall_wrx6 tools/syscall_z8
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

tools/syscall_super8: tools/syscall_super8.c ../Kernel/include/syscall_name.h
	$(CC) $(CFLAGS) -o $@ $<

tools/syscall_z8: tools/syscall_z8.c ../Kernel/include/syscall_name.h
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
ifeq ($(USERCPU),8080)
	mkdir -p /opt/fcc/lib/8080/include/arpa
	mkdir -p /opt/fcc/lib/8080/include/net /opt/fcc/lib/8080/include/netinet
	mkdir -p /opt/fcc/lib/8080/include/readline
	mkdir -p /opt/fcc/lib/8080/include/sys
	install -m 0644 include/*.h /opt/fcc/lib/8080/include
	install -m 0644 include/arpa/*.h /opt/fcc/lib/8080/include/arpa
	install -m 0644 include/net/*.h /opt/fcc/lib/8080/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/lib/8080/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/lib/8080/include/readline
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/lib/8080/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/lib/8080/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 include/sys/*.h /opt/fcc/lib/8080/include/sys
endif
ifeq ($(USERCPU),8085)
	mkdir -p /opt/fcc/lib/8085/include/arpa
	mkdir -p /opt/fcc/lib/8085/include/net /opt/fcc/lib/8085/include/netinet
	mkdir -p /opt/fcc/lib/8085/include/readline
	mkdir -p /opt/fcc/lib/8085/include/sys
	install -m 0644 include/*.h /opt/fcc/lib/8085/include
	install -m 0644 include/arpa/*.h /opt/fcc/lib/8085/include/arpa
	install -m 0644 include/net/*.h /opt/fcc/lib/8085/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/lib/8085/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/lib/8085/include/readline
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/lib/8085/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/lib/8085/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 include/sys/*.h /opt/fcc/lib/8085/include/sys
endif
ifeq ($(USERCPU),z80)
	mkdir -p /opt/fcc/lib/z80/include/arpa
	mkdir -p /opt/fcc/lib/z80/include/net /opt/fcc/lib/z80/include/netinet
	mkdir -p /opt/fcc/lib/z80/include/readline
	mkdir -p /opt/fcc/lib/z80/include/sys
	install -m 0644 include/*.h /opt/fcc/lib/z80/include
	install -m 0644 include/arpa/*.h /opt/fcc/lib/z80/include/arpa
	install -m 0644 include/net/*.h /opt/fcc/lib/z80/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/lib/z80/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/lib/z80/include/readline
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/lib/z80/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/lib/z80/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 include/sys/*.h /opt/fcc/lib/z80/include/sys
endif
ifeq ($(USERCPU),ez80_z80)
	mkdir -p /opt/fcc/lib/z80/include/arpa
	mkdir -p /opt/fcc/lib/z80/include/net /opt/fcc/lib/z80/include/netinet
	mkdir -p /opt/fcc/lib/z80/include/readline
	mkdir -p /opt/fcc/lib/z80/include/sys
	install -m 0644 include/*.h /opt/fcc/lib/z80/include
	install -m 0644 include/arpa/*.h /opt/fcc/lib/z80/include/arpa
	install -m 0644 include/net/*.h /opt/fcc/lib/z80/include/net
	install -m 0644 include/netinet/*.h /opt/fcc/lib/z80/include/netinet
	install -m 0644 include/readline/*.h /opt/fcc/lib/z80/include/readline
	install -m 0644 ../Kernel/include/drivewire.h /opt/fcc/lib/z80/include/sys/
	install -m 0644 ../Kernel/include/userstructs.h /opt/fcc/lib/z80/include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 include/sys/*.h /opt/fcc/lib/z80/include/sys
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
ifeq ($(USERCPU),z8)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),super8)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),armm4)
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
ifeq ($(USERCPU),esp8266)
	install -m 0644 ../Kernel/include/userstructs.h include/sys/
	install -m 0644 ../Kernel/include/drivewire.h include/sys/
endif
