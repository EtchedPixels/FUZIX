aaa
aad
aam
aas
adc	bx,[esi*4]
add	bx,[esi*4]
and	bx,[esi*4]
arpl	[esi*4],bx
bound	bx,[esi*4]
bsf	bx,[esi*4]
bsr	bx,[esi*4]
bswap	ebx
bt	[esi*4],bx
btc	[esi*4],bx
btr	[esi*4],bx
bts	[esi*4],bx
call	[esi*4]
cbw
cwde
clc
cld
cli
clts
cmc
cmp	bx,[esi*4]
cmpsb
cmpsw
cmpsd
cmpxchg	[esi*4],bx
cwd
cdq
daa
das
dec	[esi*4]
div	[esi*4]
enter	0x200,3
hlt
idiv	[esi*4]
imul	[esi*4]
in	al,0x20
inc	[esi*4]
insb
insw
insd
int	0x20
into
invd
invlpg	[esi*4]
iret
iretd
jnz	many
many:
jmp	[esi*4]
lahf
lar	bx,[esi*4]
lea	bx,[esi*4]
leave
lgdt	[esi*4]
lidt	[esi*4]
lds	bx,[esi*4]
les	bx,[esi*4]
lfs	bx,[esi*4]
lgs	bx,[esi*4]
lss	bx,[esi*4]
lldt	[esi*4]
lmsw	[esi*4]
lock
lodsb
lodsw
lodsd
loop	alot
alot:
lsl	bx,[esi*4]
ltr	[esi*4]
mov	ax,[esi*4]
mov	bx,[esi*4]
mov	cr0,eax
movsb
movsw
movsd
movsx	bx,byte [esi*4]
movzx	bx,byte [esi*4]
mul	[esi*4]
neg	[esi*4]
nop
not	[esi*4]
or	bx,[esi*4]
out	0x20,al
outsb
outsw
outsd
pop	[esi*4]
popa
popad
popf
popfd
push	[esi*4]
pusha
pushad
pushf
pushfd
rcl	[esi*4],1
rcr	[esi*4],1
rol	[esi*4],1
ror	[esi*4],1
rep
ins
rep
lock
outs
rep
movs
rep
cmps
rep
dseg
stos
rep
scas
repe
seg ss
ins
repe
outs
repe
lock
seg ss
movs
repe
cmps
repe
stos
repe
seg ss
seg cs
fseg
scas
repz
lock
lock
ins
repz
outs
repz
movs
repz
cmps
repz
stos
repz
scas
repne
scas
repne
cmps
repnz
scas
repnz
cmps
repnz
lock
cmps
repnz
seg ss
cmps
repnz
cseg
cmps
repnz
seg ss
lock
cmps
repnz
lock
cseg
cmps
ret
retf
sahf
sal	[esi*4],1
sar	[esi*4],1
shl	[esi*4],1
shr	[esi*4],1
sbb	bx,[esi*4]
scasb
scasw
scasd
setnz	byte [esi*4]
sgdt	[esi*4]
sidt	[esi*4]
shld	[esi*4],bx,1
shrd	[esi*4],bx,1
sldt	[esi*4]
smsw	[esi*4]
stc
std
sti
stosb
stosw
stosd
str	[esi*4]
sub	bx,[esi*4]
test	bx,[esi*4]
verr	[esi*4]
verw	[esi*4]
wait
wbinvd
xadd	[esi*4],bx
xchg	bx,[esi*4]
xlat
xor	bx,[esi*4]
