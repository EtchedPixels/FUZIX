export CROSS_AS=sdasz80
export CROSS_LD=tools/bankld/sdldz80
export CROSS_CC=sdcc
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 1000000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -mz80 -I$(ROOT_DIR)/cpu-z80 -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 200000 --opt-code-size --Werror --stack-auto --constseg CONST
export CROSS_CCOPTS=-c --std-sdcc99 --no-std-crt0 -m$(CPU) -I$(ROOT_DIR)/cpu-$(CPU) -I$(ROOT_DIR)/platform-$(TARGET) -I$(ROOT_DIR)/include --max-allocs-per-node 30000 --opt-code-size --Werror --stack-auto --constseg CONST
#export CROSS_CCOPTS+=--nostdlib --nostdinc -Isdcclib/include 
export CROSS_CC_SEG2=--codeseg CODE2
# For now but we are overspilling in a lot of configs so will need a real CODE3
export CROSS_CC_SEG3=--codeseg CODE2
export CROSS_CC_SEGDISC=--codeseg DISCARD --constseg DISCARD
export CROSS_CC_FONT=--constseg FONT
export CROSS_CC_VIDEO=--codeseg VIDEO
export ASOPTS=-plosff
export ASMEXT = .s
export BINEXT = .rel
export BITS=16
#
#	Adjust this as needed for your platform (or contribute a script
#	to look in the usual places !)
#
ifeq ($(SDCC_LIB),)
  ifeq ($(UNAME_S),Darwin)
    export LIBZ80=/usr/local/share/sdcc/lib/$(CPU)
  else
    export LIBZ80=/usr/share/sdcc/lib/$(CPU)
  endif
else
  export LIBZ80=$(SDCC_LIB)/$(CPU)
endif
