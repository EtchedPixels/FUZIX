cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fuzix C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
set(PICO_COPY_TO_RAM 1)

pico_sdk_init()

include_directories(
	.
	..
	../../dev
	../../include
	../../cpu-armm0
)

add_executable(fuzix
	devices.c
	devflash.c
	devsdspi.c
	devtty.c
	elf.c 
	main.c
	misc.c
	rawflash.c
	swapper.c
	tricks.S
	core1.c
	usbdescriptors.c
	../../dev/blkdev.c
	../../dev/mbr.c
	../../dev/devsd_discard.c
	../../dev/devsd.c
	../../lib/dhara/error.c
	../../lib/dhara/journal.c
	../../lib/dhara/map.c
	../../lib/hexdump.c
	../../blk512.c
	../../devio.c
	../../devsys.c
	../../filesys.c
	../../inode.c
	../../kdata.c
	../../cpu-armm0/lowlevel-armm0.c
	../../malloc.c
	../../mm.c
	../../memalloc_none.c
	../../process.c
	../../start.c
	../../swap.c
	../../syscall_execelf32.c
	../../syscall_exec.c
	../../syscall_fs.c
	../../syscall_fs2.c
	../../syscall_fs3.c
	../../syscall_other.c
	../../syscall_proc.c
	../../timer.c
	../../tty.c
	../../usermem.c
	../../version.c
)

target_link_libraries(fuzix
	pico_stdlib
	pico_multicore
	hardware_flash
	hardware_spi
	hardware_uart
	tinyusb_device_unmarked
	tinyusb_board
)

pico_set_float_implementation(fuzix none)
pico_set_double_implementation(fuzix none)
pico_add_extra_outputs(fuzix)

