#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <timer.h>
#include <devsd.h>
#include <stdbool.h>

#define NUM_DEV_SD 31

uint8_t sd_blockdev_count=0;

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
	return (0);
}

int sd_write(uint8_t minor, uint8_t rawflag, uint8_t flag){
	flag; minor; rawflag;
	return (0);
}

int sd_init(){

	return (0);
}

