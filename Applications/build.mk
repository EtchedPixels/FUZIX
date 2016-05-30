$(call find-makefile)

# List of one-source-file applications in util.

util_apps := \
	banner basename bd cal cat chgrp chmod chown cksum cmp cp cut date dd \
	decomp16 df dirname dosread du echo ed factor false fdisk fgrep fsck \
	grep head id init kill ll ln logname ls man mkdir mkfs mkfifo mknod \
	mount more mv od pagesize passwd patchcpm printenv prtroot ps pwd reboot rm rmdir \
	sleep ssh sort stty sum su sync tee tail telinit termcap tget touch tr true umount \
	uniq uud uue wc which while1 who whoami write xargs yes fforth \

# ...and in V7/cmd.

cmd_apps := \
	ac at atrun col comm cron crypt dc dd deroff diff3 diff diffh \
	join makekey mesg newgrp pr ptx rev split su sum test time tsort \
	wall

# These don't build:
# accton look

# ...and in V7/games.

games_apps := \
	arithmetic backgammon fish wump 

# Given an app name in $1 and a path in $2, creates a target-exe module.

make_single_app = \
	$(eval $3-$1.srcs = $2/$1.c) \
	$(call build, $3-$1, target-exe) \
	$(eval apps += $($3-$1.result))

apps :=
$(foreach app, $(util_apps), $(call make_single_app,$(app),util,util))
$(foreach app, $(cmd_apps), $(call make_single_app,$(app),V7/cmd,v7-cmd))
$(foreach app, $(games_apps), $(call make_single_app,$(app),V7/games,v7-games))

apps: $(apps)
.PHONY: apps

