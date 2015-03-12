d := $(HOSTOBJ)/Kernel/tools/bankld

SDLDZ80 = $d/sdldz80

$(SDLDZ80): \
	$d/lk_readnl.o $d/lkaomf51.o $d/lkar.o $d/lkarea.o $d/lkdata.o \
	$d/lkelf.o $d/lkeval.o $d/lkhead.o $d/lklex.o $d/lklib.o $d/lklibr.o \
	$d/lklist.o $d/lkmain.o $d/lkmem.o $d/lknoice.o $d/lkout.o $d/lkrel.o \
	$d/lkrloc.o $d/lkrloc3.o $d/lks19.o $d/lksdcclib.o $d/lksym.o $d/sdld.o \
	$d/lksdcdb.o $d/lkbank.o

$d/%.o: \
	HOSTCFLAGS += -Wno-parentheses -DINDEXLIB -DUNIX -I$(TOP)/Kernel/tools/bankld

