TOP = .
BUILD = $(TOP)/Build

OBJ = .obj
hide = @

CFLAGS = -g -O
LDFLAGS = -g

all:

include $(BUILD)/_head.mk
include $(TOP)/Standalone/build.mk
include $(BUILD)/_tail.mk

all: $(standalones)

