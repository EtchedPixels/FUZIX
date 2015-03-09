d := $(HOSTOBJ)/Standalone

$d/chmem: $d/chmem.o
$d/mkfs: $d/mkfs.o $d/util.o
$d/fsck: $d/fsck.o $d/util.o
$d/size: $d/size.o
$d/ucp: $d/ucp.o $d/util.o $d/devio.o $d/xfs1.o $d/xfs1a.o $d/xfs1b.o $d/xfs2.o

# Parse the filesystem creation script and get the list of binaries to be
# included.
binaries := $(shell cat $(TOP)/Standalone/filesystem-src/ucp-script.txt)
binaries := $(subst ../../Applications, Applications, $(binaries))
binaries := $(filter Applications/%, $(binaries))
binaries := $(patsubst %, $(OBJ)/%, $(binaries))

$(FILESYSTEM): $(TOP)/Standalone/filesystem-src/ucp-script.txt \
		$d/ucp $d/mkfs $(binaries)
	@echo FILESYSTEM $@
	@mkdir -p $(dir $@)
	$(hide) rm -f $@
	$(hide) $d/mkfs $@ 64 2880
	$(hide) sed \
		-e 's!usr-files!$(TOP)/Standalone/filesystem-src/usr-files!g' \
		-e 's!etc-files!$(TOP)/Standalone/filesystem-src/etc-files!g' \
		-e 's![./]*Applications!$(OBJ)/Applications!g' \
		$(TOP)/Standalone/filesystem-src/ucp-script.txt \
		| $d/ucp $@ > /dev/null

install:: $d/chmem $d/mkfs $d/fsck $d/size $d/ucp
	@echo INSTALL standalone tools
	$(hide) mkdir -p $(PREFIX)/bin
	$(hide) cp $^ $(PREFIX)/bin
