/*
 *	Headers. These blocks carry the structure of the program outside of
 *	the expressions
 */


#include <unistd.h>
#include <stdlib.h>
#include "compiler.h"

void header(unsigned htype, unsigned name, unsigned data)
{
	struct header h;
	h.h_type = htype;
	h.h_name = name;
	h.h_data = data;

	out_block("%H", 2);	/* For now until it's all headers/expr */
	out_block(&h, sizeof(h));
}

void footer(unsigned htype, unsigned name, unsigned data)
{
	header(htype | H_FOOTER, name, data);
}

void rewrite_header(unsigned long pos, unsigned htype, unsigned name, unsigned data)
{
	unsigned long curr = out_tell();
	out_seek(pos);
	header(htype, name, data);
	out_seek(curr);
}

unsigned long mark_header(void)
{
	return out_tell();
}
