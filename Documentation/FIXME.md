* Applications/BCPL/icint.c:443:check if
* Applications/BCPL/iclink.c:236:check if
* Applications/MWC/cmd/find.y:20:*/
* Applications/MWC/cmd/asz80/as.h:129:- sort general first
* Applications/MWC/cmd/asz80/as.h:423:- sort general first
* Applications/MWC/cmd/asz80/as1-6809.c:140:figure out extbyte etc  */
* Applications/MWC/cmd/asz80/as1-nova.c:237:needs to be encoded as an 8bit PCREL
* Applications/MWC/cmd/asz80/as1-nova.c:406:*/
* Applications/MWC/cmd/asz80/ld.c:257:        strlcpy(s->name, id, 16); */
* Applications/MWC/cmd/tar.c:904:OVERFLOW ? */
* Applications/SmallC/6809/sdiv.s:27:- self signal
* Applications/SmallC/code6801.c:262:DE cases */
* Applications/SmallC/code6801.c:515:*/
* Applications/SmallC/code8080.c:452:*/
* Applications/SmallC/codegeneric.c:416:- interaction with register needs thought */
* Applications/SmallC/codez80.c:499:*/
* Applications/SmallC/function.c:218:*/
* Applications/SmallC/main.c:93:when we do types!! */
* Applications/SmallC/primary.c:274:*/
* Applications/SmallC/sym.c:214:arrays of char only */
* Applications/V7/cmd/sh/glob.c:13:*/
* Applications/V7/cmd/sh/glob.c:40:*/
* Applications/V7/cmd/sh/xec.c:161:*/
* Applications/V7/cmd/sh/xec.c:217:*/
* Applications/V7/cmd/sh/fault.c:61:*/
* Applications/V7/cmd/sh/io.c:35:review */
* Applications/V7/cmd/sh/service.c:272:*/
* Applications/V7/cmd/ed.c:903:check lseek */
* Applications/as09/genobj.c:446:: on 16bit need to force maths to 32bit as well ? */
* Applications/cave/english.c:119:use powers of 2 ! */
* Applications/cpm/runcpm.c:34:? */
* Applications/games/qrun.c:1034:*/
* Applications/games/qrun.c:1039:*/
* Applications/games/qrun.c:1167:- via game output.. */
* Applications/games/qrun.c:1835:if (has_gfx ... ) else */
* Applications/rpilot-1.4.2/cmds.c:159:....maybe
* Applications/util/fdisk.c:68:- Should make this more flexible */
* Applications/util/fdisk.c:231:- Should make this more flexible */
* Applications/util/fdisk.c:342:/--- gotta be a better way to do this ---\ */
* Applications/util/fdisk.c:376:- Should be some value from a header */
* Applications/util/fdisk.h:22:- should be a #define'd number from header file */
* Applications/util/ln.c:77:*/
* Applications/util/mkfs.c:53:- wire up */
* Applications/util/setdate.c:133:-1 once we have dst fixed */
* Applications/util/setdate.c:229:-1 once we have dst fixed */
* Applications/util/sort.c:80:*/
* Applications/netd/slip.c:82:*/
* Applications/v7yacc/y1.c:217:VARARG*/ fprintf(stderr, s);
* Applications/v7yacc/y2.c:943:0 or EOF ??? */
* Kernel/dev/devscsi.h:115: */
* Kernel/dev/net/net_at.c:217:- screen +++ handling ! */
* Kernel/dev/vdp1.s:123:
* Kernel/dev/z80pack/devfd.c:100:We ought to cache the geometry
* Kernel/include/kernel.h:313:- wire up */
* Kernel/include/tty.h:184:add TTYF_SLEEPING 8 here so can generalize code */
* Kernel/lowlevel-68hc11.s:202:copy over)
* Kernel/platform-68hc11test/config.h:19:*/
* Kernel/platform-68hc11test/config.h:36:set to serial port for debug ease */
* Kernel/platform-68hc11test/eeprom.def:103:- useless ! .equ setbank			, 0xFF55
* Kernel/platform-68hc11test/eeprom.s:93:- .equ for this)
* Kernel/platform-68hc11test/eeprom.s:487:- optimise for 2 bytes at a time ?
* Kernel/platform-68hc11test/kernel.def:9:- tmp1 should be in eeprom RAM
* Kernel/platform-atarist/config.h:14:*/
* Kernel/platform-atarist/config.h:31:set to serial port for debug ease */
* Kernel/platform-atarist/main.c:33:*/
* Kernel/platform-coco2/mini_ide.c:33:- slices of about 4MB might be saner! */
* Kernel/platform-coco3/config.h:26:*/
* Kernel/platform-coco3/config.h:63:set to serial port for debug ease */
* Kernel/platform-dragon-nx32/build.mk:69:(and also text1/text2)
* Kernel/platform-dragon-nx32/config.h:57:set to serial port for debug ease */
* Kernel/platform-dragon/config.h:17:*/
* Kernel/platform-dragon/config.h:19:*/
* Kernel/platform-dragon/config.h:54:set to serial port for debug ease */
* Kernel/platform-dragon/setup.s:39:- DD or DB needed to get it right
* Kernel/platform-micropack/z80pack.s:168:load both as sdasz80 sulks
* Kernel/platform-msp430fr5969/config.h:57:set to serial port for debug ease */
* Kernel/platform-msx1/config.h:36:set to serial port for debug ease */
* Kernel/platform-msx2/config.h:42:set to serial port for debug ease */
* Kernel/platform-mtx/floppy.s:337:- motor bit
* Kernel/platform-multicomp09/config.h:28:*/
* Kernel/platform-multicomp09/config.h:46:set to serial port for debug ease */
* Kernel/platform-nc100/nc100.s:369:- do we need the di/ei still
* Kernel/platform-pcw8256/pcw8256.s:102:100Hz timer on
* Kernel/platform-pcw8256/devfd.c:52:   while(!(fd_send(0x04, minor) & 0x20));
* Kernel/platform-plus3/floppy.s:151:per drive
* Kernel/platform-plus3/main.c:20:floppy_timer();
* Kernel/platform-px4plus/crt0.s:52:set this
* Kernel/platform-px4plus/devfd.c:178:*/
* Kernel/platform-px4plus/sio.c:20:*/
* Kernel/platform-socz80/ethernet.c:440:*/
* Kernel/platform-socz80/devrd.c:60:Should be able to avoid the __critical once bank switching is fixed */
* Kernel/platform-tgl6502/commonmem.s:36:- we stash sp twice right now)
* Kernel/platform-tgl6502/config.h:27:*/
* Kernel/platform-ubee/devfd.c:75:*/
* Kernel/platform-ubee/devhd.c:46:*/
* Kernel/platform-ubee/devhd.c:56:*/
* Kernel/platform-zx128/config.h:58:- configure and probe */
* Kernel/platform-zx128/devfd.c:47:fd_map support
* Kernel/tools/bbc.c:202:? */
* Kernel/bank8086.c:107:once debugged #define this */
* Kernel/buddy.c:64:but will do for now */
* Kernel/buddy.c:227:*/
* Kernel/buddy.c:268:TODO */
* Kernel/buddy.c:331:*/
* Kernel/devio.c:309:*/
* Kernel/flat_mem.c:247:*/
* Kernel/flat_mem.c:276:still need to check va
* Kernel/lowlevel-65c816.s:147:check if Y too large
* Kernel/lowlevel-65c816.s:476:save the right registers to return correctly if caught
* Kernel/lowlevel-6809.s:578:temporary hack until we fix gcc-6809 or our use of it
* Kernel/lowlevel-6809.s:617:temporary hack until we fix gcc-6809 or our use of it
* Kernel/lowlevel-8086.S:383:do we need ? */
* Kernel/lowlevel-8086.S:476:*/
* Kernel/lowlevel-z80.s:152:we don't I tihnk need to save bc/de/hl
* Kernel/lowlevel-z80.s:177:use LDI x 8
* Kernel/platform-appleiie/commonmem.s:36:- we stash sp twice right now)
* Kernel/platform-appleiie/tricks.s:94:- need the extra logic to swap out the kernel Z/S bits of
* Kernel/platform-appleiie/tricks.s:147:teach swapper about 6502 S/Z
* Kernel/platform-coco2cart/mini_ide.c:46:- slices of about 4MB might be saner! */
* Kernel/platform-ibmpc/8259a.c:66:   hook_irqvec(i);
* Kernel/platform-ibmpc/config.h:41:set to serial port for debug ease */
* Kernel/platform-ibmpc/ibmpc.S:353:align buffer cache
* Kernel/platform-pdp11/config.h:33:set to serial port for debug ease */
* Kernel/platform-v65/commonmem.s:36:- we stash sp twice right now)
* Kernel/platform-v65c816-big/config.h:58:- support a
* Kernel/platform-v65c816/kernel.def:21:clashes with end of stack banks
* Kernel/platform-v68-banked/config.h:37:set to serial port for debug ease */
* Kernel/platform-v68-banked/tricks.S:75:- optimise 1K copy
* Kernel/platform-v68-banked/tricks.S:217:- optimise 1K copy
* Kernel/platform-v68-softmmu/config.h:21:set to serial port for debug ease */
* Kernel/platform-v68-softmmu/main.c:8:don't hard code! */
* Kernel/platform-v68/config.h:34:set to serial port for debug ease */
* Kernel/select.c:161:*/
* Kernel/select.c:218:lock against time race */
* Kernel/start.c:313:*/
* Kernel/syscall_net.c:136:*/
* Kernel/syscall_net.c:324:- do we want a pipedev aka Unix ? */
* Kernel/syscall_other.c:411:*/
* Kernel/tty.c:23:*/
* Library/include/math.h:103:atof equivalence */
* Library/libs/ashlsi3_6809.s:8:temporary hack until we fix gcc-6809 or our use of it
* Library/libs/ceil.c:31:signed shift
* Library/libs/curses/prntscan.c:9:*/
* Library/libs/inet_ntop.c:19:*/
* Library/libs/readdir.c:43:*/
* Library/libs/resolv.c:968:and brk this */
* Standalone/chmem.c:30:: add 6809 but remember its big endian! */
