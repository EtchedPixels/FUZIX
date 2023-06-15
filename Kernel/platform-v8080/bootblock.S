;
;	8080 cpmsim loads the first (128 byte) sector from the disk into memory
;	at 0 then executes it.	We are a bit tight on space here especially
;	in 8080
;
;	Floppy loader: 
;	Our boot disc is 77 tracks of 26 x 128 byte sectors, and we put
;	the OS on tracks 58+, which means we can put a file system in the
;	usual place providing its a bit smaller than a whole disc.
;
		.code

#define console	1
#define drive	10
#define track	11
#define sector	12
#define command	13
#define status	14
#define dmal	15
#define dmah	16
#define sectorh	17

diskload:
		di
		mvi a,'B'
		out 1
		mvi a,':'
		out 1
		xra a
		out sectorh		; sector high always 0
		out drive		; drive always 0
		mvi a,57		; start on track 58
		out track
		mvi c,19		; number of tracks to load (56Kish)

		lxi d,128
		lxi h,0x100

load_tracks:	in track
		inr a			; next track
		out track
		xra a			; sector 0 (first will be 1)
		out sector
		mvi b,26		; sectors per track
load_sectors:	
		in sector
		inr a
		out sector		; next sector
		mov a, l
		out dmal		; dma low
		mov a, h
		out dmah		; dma high
		xra a			; read
		out command		; go
		in status		; status

		mvi a,'.'
		out 1

		dad d
		dcr b
		jnz load_sectors	; 26 sectors = 3328 bytes

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
		