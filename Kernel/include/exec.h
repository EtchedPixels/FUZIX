/*
 *	Load formats (16bit only here for now
 */
 
#ifndef _SYS_EXEC_H
#define _SYS_EXEC_H

/* 16 byte header for current old style binary */
struct exec {
	uint8_t a_bra[3];		/* 3 bytes of op code */
	uint8_t a_magic[4];		/* FZX1 */
	uint8_t a_base;			/* Load address page */
	uint16_t a_size;		/* Binary memory request 0 = all */
	uint16_t a_text;
	uint16_t a_data;
	uint16_t a_bss;
	/* This isn't really part of the header but a location fixed after
	   it */
	/* uint16_t a_sigvec; */
};


#endif
