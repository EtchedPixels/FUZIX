# 0 "bootblock.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "bootblock.S"
;
; 8080 cpmsim loads the first (128 byte) sector from the disk into memory
; at 0 then executes it. We are a bit tight on space here especially
; in 8080
;
; Floppy loader:
; Our boot disc is 77 tracks of 26 x 128 byte sectors, and we put
; the OS on tracks 58+, which means we can put a file system in the
; usual place providing its a bit smaller than a whole disc.
;
  .code
# 23 "bootblock.S"
diskload:
  di
  mvi a,'B'
  out 1
  mvi a,':'
  out 1
  xra a
  out 17 ; 12 high always 0
  out 10 ; 10 always 0
  mvi a,57 ; start on 11 58
  out 11
  mvi c,19 ; number of tracks to load (56Kish)

  lxi d,128
  lxi h,0x100

load_tracks: in 11
  inr a ; next 11
  out 11
  xra a ; 12 0 (first will be 1)
  out 12
  mvi b,26 ; sectors per 11
load_sectors:
  in 12
  inr a
  out 12 ; next 12
  mov a, l
  out 15 ; dma low
  mov a, h
  out 16 ; dma high
  xra a ; read
  out 13 ; go
  in 14 ; 14

  mvi a,'.'
  out 1

  dad d
  dcr b
  jnz load_sectors ; 26 sectors = 3328 bytes

  mvi a, 13
  out 1
  mvi a,10
  out 1

  dcr c
  jnz load_tracks


  mvi a,'G'
  out 1
  mvi a, 13
  out 1
  mvi a,10
  out 1
  jmp 0x100
