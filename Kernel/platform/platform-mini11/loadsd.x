/* 
   Based on the script provided with binutils:

   Copyright (C) 2014 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
/* Linker script for 68HC11 executable (RAM).  */
MEMORY
{
    ram   (rwx) : ORIGIN = 0x0000, LENGTH = 0x0100
}

OUTPUT_ARCH(m68hc11)

/* Setup the stack on the top of the data memory bank.  */
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
  /* Start of text section.  */
  .stext   :
  {
    *(.stext)
  }  > ram
  /* Concatenate .page0 sections.  Put them in the page0 memory bank
     unless we are creating a relocatable file.  */
  .page0 :
  {
    *(.page0)
    *(.softregs)
  }  > ram
  /* This  is a hack. ld has no way to specify 'all-but' except by order,
     which in turn means we can't just put discard after bss as we'd like
     but have to fix the position */
  .init	  :
  {
    *(.init)
  } =0
  .text  :
  {
    /* Put startup code at beginning so that _start keeps same address.  */
    /* Startup code.  */
    *(.init)
    *(.text)
    *(.text.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t.*)
    *(.tramp)
    *(.tramp.*)
    /* Finish code.  */
    _etext = .;
    PROVIDE (etext = .);
  }  > ram

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
