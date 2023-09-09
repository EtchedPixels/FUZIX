Operations Used

shleq		<<=		size
shreq		>>=		SIZE, SIGN
pluseq		+=		SIZE (returns new value)
postinc		+=		SIZE (returns old value)
minuseq		-=		SIZE (returns new value)
postdec		-=		SIZE (returns old value)
cceq		==		SIZE
ltlt		<<		size
gtgt		>>		SIZE, SIGN
diveq		%=		SIZE, SIGN
muleq		*=		SIZE
xoreq		^=		SIZE
ccne		!=		SIZE
oreq		|=		SIZE
andeq		&=		SIZE
remeq		%=		SIZE
band		&		size
mul		*		size
div		/		SIZE,SIGN
rem		%		SIZE,SIGN
plus		+		size
minus		-		size
xor		^		size
cclt		<		SIZE,SIGN
ccgt		>		SIZE,SIGN
cclteq		<=		SIZE,SIGN
ccgteq		>=		SIZE,SIGN
or		|		size
cpl		~		size
not		!		SIZE
assign		=		SIZE  [*accum = tos]
deref		*		SIZE [accum = *accum]
negate		-		SIZE?, unary
funccall	()		call *accum
label				const string address
cast		(cast)		SIZE IN, SIZE OUT, SIGN OUT
constant			SIZE, constant value to stack
pop				pop accum from stack, SIZE
push				push accum to stack, SIZE
bool				boolify accum, SIZE
loadn				address of a global
loadl				address of a local
argument			address of an argument

Code generators 

argument		becomes	a loadl
enter				n stack adjust
exit				n stack adjust
cleanup				n stack adjust

jump	addr
jfalse	addr
jtrue	addr
switch				SIZE
[table]

(all these are SIZE)
nref				load a global
lbref				load a literal
lbref				load a local (arg or local)
nstore				store global
lbstore				literal
lstore				local


loadl				get address of local or argument
callname			call function by name directly


Can eliminate

any blaheq	(becomes lref op lstore - or n or lb..)
postinc/postdec		push lref op lstore pop

ccne		not cceq
cclteq		not ccgt
ccgteq		not cclt
cpl		push constant FFFF xor
argument	loadl with bias

code gen nref/lbref etc can all use loadn loadl label

callnamme is an optimization
 8 to 16bit on load (inc sign extend)

Minimal set becomes

0	ltlt		16	32
2	gtgt		16u	32u	16s	32s
6	cceq		16	32
8	band		16	32
10	mul		16	32
12	div		16u	32u	16s	32s
16	rem		16u	32u	16s	32s
20	plus		16	32
22	minus		16	32
24	xor		16	32
26	cclt		16s	32s	16u	32u
30	ccgt		16s	32s	16u	32u
34	or		16	32
36	not		16	32	(can be bool const 1 xor ?)
38	assign		8	16	32
41	deref		8s	8u	16	32
45	negate		16	32
47	funccall
48	cast8_16s
49	cast16_32s
50	constant	8s	8u	16	32 	inc name/label
54	pop		16	32
56	push		16	32
58	bool		16	32
60	loadl		; address of a local/argument (const used for other)
61	spmod		; adjust stack by 16bit signed
62	exit
63	jump
64	jfalse
65	jtrue
66	switch		8	16	32

And we also nneed
69	native	[op]		go to native code (or in pure vm call explicit native functions)

for speed nice to have

load/store direct for const (name/lit) and sp rel (local/arg)
	8/16/32 (and 8s 8u for char if we do explicitly) (or loadl8)
push const
call const
offset load 	8s	8u	16	32



68 ops

Set up via jump table 3 bytes/entry so 220 bytes

native ops
	memcpy
	memmset
	str funcs ?

for kernel
	interbank copy
	char in / noblock
	char out / noblock
	diskread disk lba bank mem
	diskwrite disk lba bank mem
	setbank
	getrtc

How to do virtualized interrupt ?

	inttimer	- set interrupt timer to A
	setsp		- set sp (can get sp with local 0)

Int handler just pushes old pc sets pc to one selected caller can then do

	push
	loadl 0
	store somewhere
	new sp into a
	setsp
	blah blah
	load old sp into a
	setsp
	pop
	exit

	