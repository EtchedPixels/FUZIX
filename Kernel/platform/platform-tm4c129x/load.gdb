target remote :3333
monitor arm semihosting enable
load
tbreak start
monitor reset halt
continue
