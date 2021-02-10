cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(fuzix C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

pico_sdk_init()
include_directories(
	.
	../include
	../cpu-armm0
)

add_executable(fuzix
	main.c
	../start.c
)

target_link_libraries(fuzix pico_stdlib)
pico_add_extra_outputs(fuzix)

