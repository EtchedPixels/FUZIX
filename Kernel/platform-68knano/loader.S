#define DATA	0
#define ERR	3
#define NSEC	5
#define LBA0	7
#define LBA1	9
#define LBA2	11
#define LBA3	13
#define COMMAND 15
#define STATUS  15

#define READY	6
#define DRQ	3

#define READ	$20


#define THRA	7
#define SRA	3

/*
 *	Simple loader to get us going
 */
	.globl start
	.globl loader

.mri	1

start:
	jmp.l $15050
	jmp.l $15050		; any value will do
	; This bit fools the CP/M bootloader checking
	; 34 words of padding
	.ds 34
	; should be where we end up from the bra
loader:
	or #$0700,sr
	lea $001F0000,a0
	lea $00FFF000,a2
	lea bootmsg,a1
txout:
	tst.b (a1)
	beq now_reloc
waitc:
	btst.b #2,SRA(a2)
	beq waitc
	move.b (a1)+,THRA(a2)
	bra txout

bootmsg:
	ascii "Not loading CP/M but FUZIX"
	byte 13,10,0

	even

now_reloc
	lea strap,a1
	move.w #1024,d0
reloc:
	move.w (a1)+,(a0)+
	dbra d0,reloc
	jmp.l $001F0000

strap:
	move.b #10,THRA(a2)
	lea $00FFE000,a1
	lea $00000400,a0
	move.b #2,d2			; Sector
	move.b #$80,d1			; Count to load (64K for now)
	move.b #$E0,LBA3(a1)
wait0:
	btst #READY,STATUS(a1)
	beq wait0

	move.b #0,LBA2(a1)
	move.b #0,LBA1(a1)
load:
	move.b d2,LBA0(a1)
	move.b #1,NSEC(a1)
	add.q #1,d2
wait1:
	btst #READY,STATUS(a1)
	beq wait1
	move.b #READ,COMMAND(a1)
	nop
wait2:
	btst #DRQ,STATUS(a1)
	beq wait2

	move.b #'.',THRA(a2)
	move.w #255,d0
sector:
	move.w (a1),(a0)+
	dbra d0,sector
	dbra d1,load

	move.b #13,THRA(a2)
waitc2:
	btst.b #2,SRA(a2)
	beq waitc2
	move.b #10,THRA(a2)
	jmp.l $00000404
