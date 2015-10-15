$(call find-makefile)

chmem.srcs = chmem.c
$(call build, chmem, host-exe)

size.srcs = size.c
$(call build, size, host-exe)

mkfs.srcs = mkfs.c util.c
$(call build, mkfs, host-exe)

fsck.srcs = fsck.c util.c
$(call build, fsck, host-exe)

ucp.srcs = ucp.c util.c
$(call build, ucp, host-exe)

standalones: chmem size mkfs fsck ucp
.PHONY: standalones

