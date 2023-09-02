CROSS_CCOPTS += -I$(ROOT_DIR)/platform-amstradnc -DCONFIG_NC200
ASOPTS += -I$(ROOT_DIR)
CROSS_CCOPTS += --peep-file $(FUZIX_ROOT)/Kernel/cpu-z80/rst.peep
