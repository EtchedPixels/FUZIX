T := $(TARGET)
O := $(OBJ)/host/$T
exe := $(O)/$T
$(eval $T := $(exe))

srcs := $(call absify, $($T.dir), $($T.srcs))
objs := $(patsubst %, $O/%.o, $(basename $(srcs)))

$(exe): $(objs)
	@echo HOSTLINK $@
	@mkdir -p $(dir $@)
	$(hide) gcc -o $@ $^ $($T.ldflags) $(LDFLAGS)

$O/%.o: %.c
	@echo HOSTCC $@
	@mkdir -p $(dir $@)
	$(hide) gcc -c -o $@ $< $($T.cflags) $(CFLAGS)

