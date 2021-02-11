MEMORY
{
  dport0_0_seg : org = 0x3ff00000, len = 0x10
  userdata_seg : org = 0x3ffe8000, len = 0x10000
  dram0_0_seg :  org = 0x3fff8000, len = 0x4000
  dram1_0_seg :  org = 0x3fffc000, len = 0x4000
  usercode_seg : org = 0x40100000, len = 0x7e00
  iram1_0_seg :  org = 0x40107e00, len = 0x200
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

/*  Default entry point:  */
ENTRY(__main)
EXTERN(_DebugExceptionVector)
EXTERN(_DoubleExceptionVector)
EXTERN(_KernelExceptionVector)
EXTERN(_NMIExceptionVector)
EXTERN(_UserExceptionVector)
PROVIDE(_memmap_vecbase_reset = 0x40000000);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00000110;
_memmap_cacheattr_wt_base = 0x00000110;
_memmap_cacheattr_bp_base = 0x00000220;
_memmap_cacheattr_unused_mask = 0xFFFFF00F;
_memmap_cacheattr_wb_trapnull = 0x2222211F;
_memmap_cacheattr_wba_trapnull = 0x2222211F;
_memmap_cacheattr_wbna_trapnull = 0x2222211F;
_memmap_cacheattr_wt_trapnull = 0x2222211F;
_memmap_cacheattr_bp_trapnull = 0x2222222F;
_memmap_cacheattr_wb_strict = 0xFFFFF11F;
_memmap_cacheattr_wt_strict = 0xFFFFF11F;
_memmap_cacheattr_bp_strict = 0xFFFFF22F;
_memmap_cacheattr_wb_allvalid = 0x22222112;
_memmap_cacheattr_wt_allvalid = 0x22222112;
_memmap_cacheattr_bp_allvalid = 0x22222222;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

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

		*(.literal .text .literal.* .text.*)
		*(.rodata .rodata.*)
		*(.rodata1 .rodata1.*)

		_irom0_text_end = ABSOLUTE(.);
	} >irom0_0_seg :irom0_0_phdr
}

