/*      File data.c: 2.2 (84/11/27,16:26:13) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"

/* storage words */
SYMBOL symbol_table[NUMBER_OF_GLOBALS + NUMBER_OF_LOCALS];
int global_table_index, rglobal_table_index;
int local_table_index;

WHILE ws[WSTABSZ];
int     while_table_index;

int     swstcase[SWSTSZ];
int     swstlab[SWSTSZ];
int     swstp;
char    macq[MACQSIZE];
int     macptr;
char    line[LINESIZE];
char    mline[LINESIZE];
int     lptr, mptr;

TAG_SYMBOL  tag_table[NUMTAG]; // start of structure tag table
int         tag_table_index; // ptr to next entry

SYMBOL	member_table[NUMMEMB];	// structure member table
int	member_table_index;	// ptr to next member

/* miscellaneous storage */
int     nxtlab,
        stkp,
        argstk,
        ncmp,
        errcnt,
        glbflag,
        ctext,
        cmode,
        lastst;

int     input, input2, output, target;
int     inclstk[INCLSIZ] = {-1, -1, -1 };
int     inclsp;
char    fname[20];
char	input_eof;

//char    quote[2];
int     current_symbol_table_idx; //char    *cptr;
int     *iptr;
int     fexitlab;
int     iflevel, skiplevel;
int     errfile;
int     cflag;
int     errs;
int     aflag;
int     uflag;  // undocumented 8085 instructions

INITIALS initials_table[NUMBER_OF_GLOBALS];
int initials_idx = 0;
