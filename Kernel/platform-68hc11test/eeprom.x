/* Modified from the linker script, for normal executables */
/* Copyright (C) 2014 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
/* Linker script for Alan's 68HC811E2  */
OUTPUT_FORMAT("elf32-m68hc11", "elf32-m68hc11",
	      "elf32-m68hc11")
OUTPUT_ARCH(m68hc11)
ENTRY(_start)
SEARCH_DIR("/usr/local/m6811-elf/lib");
/* 68HC811 memory configuration. Right now we are putting I/O at 0xF000 and
   the unbanked RAM at 0xF040-F0FF */
MEMORY
{
  page0 (rwx) : ORIGIN = 0x00000, LENGTH = 0x0100
  ram   (rwx) : ORIGIN = 0x00100, LENGTH = 0xEE00
  ports (rw)  : ORIGIN = 0x0F000, LENGTH = 0x0040
  iram  (rwx) : ORIGIN = 0x0F040, LENGTH = 0x00C0
  ram2  (rwx) : ORIGIN = 0x0F100, LENGTH = 0x0500
  eeram (rwx) : ORIGIN = 0x0F600, LENGTH = 0x0200
  eeprom (rx) : ORIGIN = 0x0F800, LENGTH = 0x0740
  eesysc (rx) : ORIGIN = 0x0FF40, LENGTH = 0x0096
  eevec (rx)  : ORIGIN = 0x0FFD6, LENGTH = 0x002A

}

SECTIONS
{
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .gnu.version		  : { *(.gnu.version) }
  .gnu.version_d	  : { *(.gnu.version_d) }
  .gnu.version_r	  : { *(.gnu.version_r) }
  .rel.text      :
    {
      *(.rel.text)
      *(.rel.text.*)
      *(.rel.gnu.linkonce.t.*)
    }
  .rela.text     :
    {
      *(.rela.text)
      *(.rela.text.*)
      *(.rela.gnu.linkonce.t.*)
    }
  .rel.data      :
    {
      *(.rel.data)
      *(.rel.data.*)
      *(.rel.gnu.linkonce.d.*)
    }
  .rela.data     :
    {
      *(.rela.data)
      *(.rela.data.*)
      *(.rela.gnu.linkonce.d.*)
    }
  .rel.rodata    :
    {
      *(.rel.rodata)
      *(.rel.rodata.*)
      *(.rel.gnu.linkonce.r.*)
    }
  .rela.rodata   :
    {
      *(.rela.rodata)
      *(.rela.rodata.*)
      *(.rela.gnu.linkonce.r.*)
    }
  .rel.sdata     :
    {
      *(.rel.sdata)
      *(.rel.sdata.*)
      *(.rel.gnu.linkonce.s.*)
    }
  .rela.sdata     :
    {
      *(.rela.sdata)
      *(.rela.sdata.*)
      *(.rela.gnu.linkonce.s.*)
    }
  .rel.sbss      :
    {
      *(.rel.sbss)
      *(.rel.sbss.*)
      *(.rel.gnu.linkonce.sb.*)
    }
  .rela.sbss     :
    {
      *(.rela.sbss)
      *(.rela.sbss.*)
      *(.rel.gnu.linkonce.sb.*)
    }
  .rel.bss       :
    {
      *(.rel.bss)
      *(.rel.bss.*)
      *(.rel.gnu.linkonce.b.*)
    }
  .rela.bss      :
    {
      *(.rela.bss)
      *(.rela.bss.*)
      *(.rela.gnu.linkonce.b.*)
    }
  .rel.stext		  : { *(.rel.stest) }
  .rela.stext		  : { *(.rela.stest) }
  .rel.etext		  : { *(.rel.etest) }
  .rela.etext		  : { *(.rela.etest) }
  .rel.sdata		  : { *(.rel.sdata) }
  .rela.sdata		  : { *(.rela.sdata) }
  .rel.edata		  : { *(.rel.edata) }
  .rela.edata		  : { *(.rela.edata) }
  .rel.eit_v		  : { *(.rel.eit_v) }
  .rela.eit_v		  : { *(.rela.eit_v) }
  .rel.ebss		  : { *(.rel.ebss) }
  .rela.ebss		  : { *(.rela.ebss) }
  .rel.srodata		  : { *(.rel.srodata) }
  .rela.srodata		  : { *(.rela.srodata) }
  .rel.erodata		  : { *(.rel.erodata) }
  .rela.erodata		  : { *(.rela.erodata) }
  .rel.got		  : { *(.rel.got) }
  .rela.got		  : { *(.rela.got) }
  .rel.ctors		  : { *(.rel.ctors) }
  .rela.ctors		  : { *(.rela.ctors) }
  .rel.dtors		  : { *(.rel.dtors) }
  .rela.dtors		  : { *(.rela.dtors) }
  .rel.init		  : { *(.rel.init) }
  .rela.init		  : { *(.rela.init) }
  .rel.fini		  : { *(.rel.fini) }
  .rela.fini		  : { *(.rela.fini) }
  .rel.plt		  : { *(.rel.plt) }
  .rela.plt		  : { *(.rela.plt) }
  /* Concatenate .page0 sections.  Put them in the page0 memory bank
     unless we are creating a relocatable file.  */
  .page0 :
  {
    *(.page0)
    *(.softregs)
  }  > page0
  /* Start of text section.  */
  .stext   :
  {
    *(.stext)
  }  > eeprom
  .init	  :
  {
    *(.init)
  } =0
  .text  :
  {
    /* Put startup code at beginning so that _start keeps same address.  */
    /* Startup code.  */
    KEEP (*(.install0))	/* Section should setup the stack pointer.  */
    KEEP (*(.install1))	/* Place holder for applications.  */
    KEEP (*(.install2))	/* Optional installation of data sections in RAM.  */
    KEEP (*(.install3))	/* Place holder for applications.  */
    KEEP (*(.install4))	/* Section that calls the main.  */
    *(.init)
    *(.text)
    *(.text.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t.*)
    *(.tramp)
    *(.tramp.*)
    /* Finish code.  */
    KEEP (*(.fini0))	/* Beginning of finish code (_exit symbol).  */
    KEEP (*(.fini1))	/* Place holder for applications.  */
    KEEP (*(.fini2))	/* C++ destructors.  */
    KEEP (*(.fini3))	/* Place holder for applications.  */
    KEEP (*(.fini4))	/* Runtime exit.  */
    _etext = .;
    PROVIDE (etext = .);
  }  > eeprom
  .eh_frame   :
  {
    KEEP (*(.eh_frame))
  }  > eeprom
  .gcc_except_table   :
  {
    *(.gcc_except_table)
  }  > eeprom
  .rodata    :
  {
    *(.rodata)
    *(.rodata.*)
    *(.gnu.linkonce.r*)
  }  > eeprom
  .rodata1   :
  {
    *(.rodata1)
  }  > eeprom
  /* Constructor and destructor tables are in text.  */
  .ctors   :
  {
     PROVIDE (__CTOR_LIST__ = .);
    KEEP (*(.ctors))
     PROVIDE(__CTOR_END__ = .);
  }  > eeprom
    .dtors	  :
  {
     PROVIDE(__DTOR_LIST__ = .);
    KEEP (*(.dtors))
     PROVIDE(__DTOR_END__ = .);
  }  > eeprom
  .jcr   :
  {
    KEEP (*(.jcr))
  }  > eeprom
  /* Start of the data section image in RAM.  */
  .data	:
  {
    *(.sdata)
    *(.data)
    *(.data.*)
    *(.data1)
    *(.gnu.linkonce.d.*)
    CONSTRUCTORS
    _edata  =  .;
    PROVIDE (edata = .);
  }  > eeram
  /* Relocation for some bss and data sections.  */
  .bss   :
  {
    __bss_start = .;
    *(.sbss)
    *(.scommon)
    *(.dynbss)
    *(.bss)
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    PROVIDE (_end = .);
  }  > eeram
  __bss_size = SIZEOF(.bss);
  PROVIDE (__bss_size = SIZEOF(.bss));
  .eeprom   :
  {
    *(.eeprom)
    *(.eeprom.*)
  }  > eeprom
  .iram   :
  {
    *(.iram)
    *(.iram.*)
  }  > iram
  .syscalls  :
  {
    *(.syscalls)
    *(.syscalls.*)
  } > eesysc
  .vectors  :
  {
    *(.vectors)
    *(.vectors.*)
  } > eevec

  /* Stabs debugging sections.  */
  .stab		 0 : { *(.stab) }
  .stabstr	 0 : { *(.stabstr) }
  .stab.excl	 0 : { *(.stab.excl) }
  .stab.exclstr	 0 : { *(.stab.exclstr) }
  .stab.index	 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment	 0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end ) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /* DWARF Extension.  */
  .debug_macro    0 : { *(.debug_macro) }
}
