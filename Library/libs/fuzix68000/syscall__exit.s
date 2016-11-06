# 1 "syscall__exit.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 1 "<command-line>" 2
# 1 "syscall__exit.S"
 .area .text

 .globl __syscall
 .globl __exit

__exit:
 move.w #0,d0
 trap #14
