$(call find-makefile)

ctype-test.srcs = ctype.c
$(call build, ctype-test, host-test)

