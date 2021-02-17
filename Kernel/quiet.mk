# Define V=1 for more verbose compilation.

V             = @
Q             = $(V:1=)
QUIET_AR      = $(Q:@=@echo    '  AR      '$@;)
QUIET_AS      = $(Q:@=@echo    '  AS      '$@;)
QUIET_CC      = $(Q:@=@echo    '  CC      '$@;)
QUIET_LD      = $(Q:@=@echo    '  LD      '$@;)
QUIET_GEN     = $(Q:@=@echo    '  GEN     '$@;)
QUIET_RM      = $(Q:@=@echo    '  RM      '$@;)
QUIET_TEST    = $(Q:@=@echo    '  TEST    '$@;)
