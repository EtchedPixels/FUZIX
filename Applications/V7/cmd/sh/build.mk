$(call find-makefile)

v7-sh.srcs = \
	args.c blok.c builtin.c cmd.c ctype.c error.c expand.c fault.c io.c \
	macro.c main.c msg.c name.c print.c service.c setbrk.c stak.c string.c \
	word.c xec.c glob.c
$(call build, v7-sh, target-exe)

apps: $(v7-sh.result)

