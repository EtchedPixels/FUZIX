$(call find-makefile)

map_syscall.srcs = map_syscall.c
$(call build, map_syscall, host-exe)

