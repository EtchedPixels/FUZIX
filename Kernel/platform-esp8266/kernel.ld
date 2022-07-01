/* The true dram1_0_seg is 3FFFC000 for 0x4000 but we need to be sure we
 * error if we touch a few bits of, so we split it up into spaces
 * representing the memory we actually have. Note that the loader has no
 * clue about this kind of abuse so all the data must be bss equvialent
 */

MEMORY
{
  dport0_0_seg : org = 0x3ff00000, len = 0x10
  userdata_seg : org = 0x3ffe8000, len = 0x10000 /* check with config.h */
  dram0_0_seg :  org = 0x3fff8000, len = 0x4000
/*  dram1_0_seg :  org = 0x3fffc000, len = 0x4000 */
  dram1_0l_seg : org = 0x3fffc040, len = 0x06d4
  dram1_0h_seg : org = 0x3fffc734, len = 0x38cc
  usercode_seg : org = 0x40100000, len = 0x7e00 /* check with config.h */
  iram1_0_seg :  org = 0x40107e00, len = 0x200
  iram1_1_seg :  org = 0x40108000, len = 0x4000
  iram1_2_seg :  org = 0x4010c000, len = 0x4000
  irom0_0_seg :  org = 0x40210000, len = 0x5C000
}

PHDRS
{
  dport0_0_phdr PT_LOAD;
  dram0_0_phdr PT_LOAD;
  dram0_0_bss_phdr PT_LOAD;
  iram0_0_phdr PT_LOAD;
  iram1_0_phdr PT_LOAD;
  irom0_0_phdr PT_LOAD;
}

_data_base = ORIGIN(userdata_seg);
_data_top = _data_base + LENGTH(userdata_seg);
_code_base = ORIGIN(usercode_seg);
_code_top = _code_base + LENGTH(usercode_seg);

PROVIDE(udata = _data_base);
PROVIDE(bufpool = 0x3fffc734);

/*  Default entry point:  */
ENTRY(__main)

SECTIONS
{

  .dport0.rodata : ALIGN(4)
  {
    _dport0_rodata_start = ABSOLUTE(.);
    *(.dport0.rodata)
    *(.dport.rodata)
    _dport0_rodata_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.literal : ALIGN(4)
  {
    _dport0_literal_start = ABSOLUTE(.);
    *(.dport0.literal)
    *(.dport.literal)
    _dport0_literal_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

  .dport0.data : ALIGN(4)
  {
    _dport0_data_start = ABSOLUTE(.);
    *(.dport0.data)
    *(.dport.data)
    _dport0_data_end = ABSOLUTE(.);
  } >dport0_0_seg :dport0_0_phdr

	.data : ALIGN(4)
	{
		_data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.data1)
		*(.data1.*)
		*(.sdata)
		*(.sdata.*)
		*(.sdata2)
		*(.sdata2.*)
		*(.jcr)
		_data_end = ABSOLUTE(.);
	} >dram0_0_seg :dram0_0_phdr

	.bss ALIGN(8) (NOLOAD) : ALIGN(4)
	{
		. = ALIGN (8);
		_bss_start = ABSOLUTE(.);
		*(.dynsbss)
		*(.sbss)
		*(.sbss.*)
		*(.gnu.linkonce.sb.*)
		*(.scommon)
		*(.sbss2)
		*(.sbss2.*)
		*(.gnu.linkonce.sb2.*)
		*(.dynbss)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		_bss_end = ABSOLUTE(.);
		_heap_start = ABSOLUTE(.);
	} >dram0_0_seg :dram0_0_bss_phdr

	/* Loaded into the iram by the boot loader. */

	.text : ALIGN(4)
	{
		_stext = .;
		_text_start = ABSOLUTE(.);
		*boot.o(.literal .text .literal.* .text.*)
		*boot.o(.rodata .rodata.*)
		_text_end = ABSOLUTE(.);
		_etext = .;
	} >usercode_seg :iram0_0_phdr

	/* Loaded into the iram by the boot loader. */

	.iram1.text : ALIGN(4)
	{
		*rawflash.o(.literal .text .literal.* .text.*)
		*rawflash.o(.rodata .rodata.*)
	} >iram1_0_seg :iram1_0_phdr

	/* Access via the memory-mapped SPI flash. */

	.irom0.text : ALIGN(4)
	{
		_irom0_text_start = ABSOLUTE(.);
		*(.text.vectors)
		*(.literal .text .literal.* .text.*)
		*(.rodata .rodata.*)
		*(.rodata1 .rodata1.*)

		_irom0_text_end = ABSOLUTE(.);
	} >irom0_0_seg :irom0_0_phdr
}

