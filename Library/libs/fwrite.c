/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */  
    
/* This is an implementation of the C standard IO package. */ 
    
#include "stdio-l.h"
    
/*
 * Like fread, fwrite will often be used to write out large chunks of
 * data; calling write() directly can be a big win in this case.
 *
 * But first we check to see if there's space in the buffer.
 */ 
int fwrite(const void *buf, size_t size, size_t nelm, FILE * fp) 
{
	int len;
	register int v;
	unsigned int bytes, put;

	if (!buf || !size || !nelm || !fp)
		return 0;
	v = fp->mode;
	
	/* If last op was a read ... */ 
	if ((v & __MODE_READING) && fflush(fp))
		return 0;
	
	/* Can't write or there's been an EOF or error then return 0 */ 
	if ((v & (__MODE_WRITE | __MODE_EOF | __MODE_ERR)) != __MODE_WRITE)
		return 0;
	
	/* This could be long, doesn't seem much point tho */ 
	bytes = size * nelm;
	len = fp->bufend - fp->bufpos;
	
	/* Flush the buffer if not enough room */ 
	if (bytes > len && fflush(fp))
		return 0;
	len = fp->bufend - fp->bufpos;
	if (bytes <= len) {	/* It'll fit in the buffer ? */
		fp->mode |= __MODE_WRITING;
		memcpy(fp->bufpos, buf, bytes);
		fp->bufpos += bytes;

		/* If we're not fully buffered */ 
		if (v & (_IOLBF | _IONBF))
			fflush(fp);
		return nelm;
	}
	
	/* Too big for the buffer */ 
	/* ??? May be leave the rest of data in buffer ? */ 
	put = bytes;
	
	do {
		if ((len = write(fp->fd, buf, bytes)) > 0) {
			buf = (char *) buf + len;
			bytes -= len;
		}
	} while (len > 0 || (len == -1 && errno == EINTR));
	if (len < 0)
		fp->mode |= __MODE_ERR;
	put -= bytes;
	return put / size;
}
