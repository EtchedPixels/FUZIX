#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>

#define NUM_DEV_SD 31


static unsigned char zsd_init()__naked;
static unsigned char zsd_cmp()__naked;
static unsigned char zsd_rdblk(unsigned char count, unsigned long nsect, void* buf)__naked;
static unsigned char zsd_wrblk(unsigned char count, unsigned long nsect, void* buf)__naked;

unsigned char sd_blockdev_count=0;

// offset block for reading (first sector of UZI image)
unsigned long part_offset=2048;

//
unsigned long part_size=32UL*1024UL*1024UL;

int sd_open(uint8_t minor, uint16_t flag){
	flag;

	if(minor < sd_blockdev_count){
        	return 0;
	} else {
		udata.u_error = EIO;
		return -1;
	}
}


int sd_close(uint8_t minor){
	minor;
	return 0;
}

int sd_read(uint8_t minor, uint8_t rawflag, uint8_t flag){
	flag; minor; rawflag;
	return (!zsd_rdblk(1, part_offset+udata.u_buf->bf_blk, udata.u_buf->bf_data));
}

int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag){
	flag; minor; rawflag;
	return (0);
}

int sd_init(){

	char present=!(zsd_init() | zsd_cmp());

	kprintf("Probe Z-Controller SD-card... %s\r\n",present?"Ok":"Not found");
	///@TODO Insert partition autodetect
	if(present){
		sd_blockdev_count++;
	}
	//
	return (0);
}


static unsigned char zsd_init()__naked{
__asm;
	call ZSD_SHAD_STORE
	call ZSD_INIT
	call ZSD_SHAD_RESTORE
	ld l,a
	ret
__endasm;
}

static unsigned char zsd_cmp()__naked{
__asm;
	call ZSD_SHAD_STORE
	call ZSD_CMP
	call ZSD_SHAD_RESTORE
	ld l,a
	ret
__endasm;
}



static unsigned char zsd_rdblk(unsigned char count, unsigned long nsect, void* buf)__naked{
	count; nsect; buf;
__asm;
	call ZSD_SHAD_STORE
	ld  iy,#0
	add iy,sp
	;// count
	ld  a,2(iy)
	;// nsect
	ld e,3(iy)
	ld d,4(iy)
	ld c,5(iy)
	ld b,6(iy)
	; // buf
	ld l,7(iy)
	ld h,8(iy)
	;
	call ZSD_RDMULTI
	call ZSD_SHAD_RESTORE
	ld l,a
	ret
__endasm;
}

static unsigned char zsd_wrblk(unsigned char count, unsigned long nsect, void* buf)__naked{
	count; nsect; buf;
__asm;
	call ZSD_SHAD_STORE
	ld  iy,#0
	add iy,sp
	;// count
	ld  a,2(iy)
	;// nsect
	ld e,3(iy)
	ld d,4(iy)
	ld c,5(iy)
	ld b,6(iy)
	; // buf
	ld l,7(iy)
	ld h,8(iy)
	;
	call ZSD_WRMULTI
	call ZSD_SHAD_RESTORE
	ld l,a
	ret
__endasm;
}

