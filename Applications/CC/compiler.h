
/* Pass 2 values */

/* This controls the number of symbols (including complex types, arrays and
   unique function prototypes. Cost is 10 bytes per node on a small box. We
   can probably make symbols the self expanding one eventually */
#define MAXSYM			768
/* Expression nodes. Currently 16 bytes on a small box will be about 24 once
   we have everything in */
#define NUM_NODES		100
/* Number of bytees of index data used for tagging structs, prototypes etc */
#define IDX_SIZE		2560
/* Maximum number of goto labels per function (not switches), 4 bytes each */
#define MAXLABEL		16
/* Maximum number of fields per structure, 6 bytes per entry on stack, per
   recursive struct definition */
#define NUM_STRUCT_FIELD	50
/* Number of switch entries within the current scope. 4 bytes per entry */
#define NUM_SWITCH		128
/* Number of constants from enum. 4 bytes per entry */
#define NUM_CONSTANT		50

#include <stdio.h>

#include "symbol.h"

#include "body.h"
#include "declaration.h"
#include "enum.h"
#include "error.h"
#include "expression.h"
#include "header.h"
#include "idxdata.h"
#include "initializer.h"
#include "label.h"
#include "lex.h"
#include "primary.h"
#include "storage.h"
#include "stackframe.h"
#include "struct.h"
#include "switch.h"
#include "target.h"
#include "token.h"
#include "tree.h"
#include "type.h"
#include "type_iterator.h"

extern FILE *debug;

extern unsigned in_sizeof;
