/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
/* This is an implementation of the C standard IO package. */ 
    
#include "stdio-l.h"
    
/*
 * This Fopen is all three of fopen, fdopen and freopen. The macros in
 * stdio.h show the other names.
 */ 

FILE * __fopen(const char *fname, int fd, FILE * fp, const char *mode) 
{
	uint open_mode = 0;
	
	int fopen_mode = 0;
	FILE * nfp = 0;
	 
	/* If we've got an fp close the old one (freopen) */ 
	if (fp) {
		/* Careful, don't de-allocate it */ 
		fopen_mode |= (fp->mode & (__MODE_BUF | __MODE_FREEFIL |__MODE_FREEBUF));
		fp->mode &= ~(__MODE_FREEFIL | __MODE_FREEBUF);
		fclose(fp);
	}
	
	/* decode the new open mode */ 
	while (*mode) {
		switch (*mode++) {
		case 'r':
			fopen_mode |= __MODE_READ;
			break;
		case 'w':
			fopen_mode |= __MODE_WRITE;
			open_mode = (O_CREAT | O_TRUNC);
			break;
		case 'a':
			fopen_mode |= __MODE_WRITE;
			open_mode = (O_CREAT | O_APPEND);
			break;
		case '+':
			fopen_mode |= __MODE_RDWR;
			break;
		}
	}

	/* Add in the read/write options to mode for open() */ 
	switch (fopen_mode & (__MODE_READ | __MODE_WRITE)) {
	case 0:
		return NULL;
	case __MODE_READ:
		open_mode |= O_RDONLY;
		break;
	case __MODE_WRITE:
		open_mode |= O_WRONLY;
		break;
	default:
		open_mode |= O_RDWR;
		break;
	}

	/* Allocate the (FILE) before we do anything irreversable */ 
	if (fp == NULL && (nfp = calloc(1, sizeof(FILE))) == NULL)
		return NULL;
	
	/* Open the file itself */ 
	if (fname)
		fd = open(fname, open_mode, 0666);
	if (fd < 0) {		/* Grrrr */
		if (nfp)
			free(nfp);
		return 0;
	}
	
	/* If this isn't freopen create a (FILE) and buffer for it */ 
	if (fp == NULL) {
		fp = nfp;
		fp->next = __IO_list;
		__IO_list = fp;	/* add to list */
		fp->mode = __MODE_FREEFIL;
		if (isatty(fd))
			fp->mode |= _IOLBF;
#if _IOFBF
		else
			fp->mode |= _IOFBF;
		
#endif
		if ((fp->bufstart = calloc(1, BUFSIZ)) == NULL) {
			/* Oops, no mem
			 * Humm, full buffering with a eight(!) byte buffer.
			 */ 
			fp->bufstart = (uchar *) fp->unbuf;
			fp->bufend = (uchar *) fp->unbuf + sizeof(fp->unbuf);
		} else {
			fp->bufend = fp->bufstart + BUFSIZ;
			fp->mode |= __MODE_FREEBUF;
		}
	}
	
	/* Ok, file's ready clear the buffer and save important bits */ 
	fp->bufpos = fp->bufread = fp->bufwrite = fp->bufstart;
	fp->mode |= fopen_mode;
	fp->fd = fd;
	return fp;
}
 
