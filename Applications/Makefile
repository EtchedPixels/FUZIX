#

all:	util cmd sh games cave cpm games ld09 netd SmallC MWC

util:
	+(cd util; $(MAKE) -f Makefile.$(USERCPU))

cmd:
	+(cd V7/cmd; $(MAKE) -f Makefile.$(USERCPU))

sh:
	+(cd V7/cmd/sh; $(MAKE) -f Makefile.$(USERCPU))

games:
	+(cd V7/games; $(MAKE) -f Makefile.$(USERCPU))

cave:
	+(cd cave; $(MAKE) -f Makefile.$(USERCPU))

cpm:
	+(cd cpm; $(MAKE) -f Makefile.$(USERCPU))

games:
	+(cd games; $(MAKE) -f Makefile.$(USERCPU))

ld09:
	+(cd ld09; $(MAKE) -f Makefile.$(USERCPU))

netd:
	+(cd netd; $(MAKE) -f Makefile.$(USERCPU))

SmallC:
	+(cd SmallC; $(MAKE) -f Makefile.$(USERCPU))

MWC:
	+(cd MWC/cmd; $(MAKE) -f Makefile.$(USERCPU))

clean:
	(cd util; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd V7/cmd; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd V7/cmd/sh; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd V7/games; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd cave; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd cpm; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd games; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd ld09; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd netd; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd SmallC; $(MAKE) -f Makefile.$(USERCPU) clean)
	(cd MWC/cmd; $(MAKE) -f Makefile.$(USERCPU) clean)

