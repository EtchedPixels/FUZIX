$(call find-makefile)

levee.srcs = \
	beep.c blockio.c display.c doscall.c editcor.c exec.c find.c \
	flexcall.c gemcall.c globals.c insert.c main.c misc.c modify.c \
	move.c rmxcall.c ucsd.c undo.c unixcall.c wildargs.c
$(call build, levee, target-exe)

apps: $(levee.result)

