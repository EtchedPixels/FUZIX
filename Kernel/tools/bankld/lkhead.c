/* lkhead.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 */

#include "aslink.h"

/*Module	lkhead.c
 *
 *	The module lkhead.c contains the function newhead() which
 *	creates a head structure, the function module() which
 *	loads the module name into the current head structure,
 *	and newmode() which loads the linker merge modes.
 *
 *	lkhead.c contains the following functions:
 *		VOID	newhead()
 *		VOID	newmode()
 *		VOID	module()
 *
 *	lkhead.c contains no local variables.
 */

/*)Function	VOID	newhead()
 *
 *	The function newhead() creates a head structure.  All head
 *	structures are linked to form a linked list of head structures
 *	with the current head structure at the tail of the list.
 *
 *	local variables:
 *		int	i		evaluation value
 *		char	id[]		temporary string
 *		head *	thp		temporary pointer
 *					to a header structure
 *
 *	global variables:
 *		area	*ap		Pointer to the current
 *					area structure
 *		lfile	*cfp		The pointer *cfp points to the
 *					current lfile structure
 *		head	*headp		The pointer to the first
 *					head structure of a linked list
 *		head	*hp		Pointer to the current
 *					head structure
 *
 *	functions called:
 *		a_uint	expr()		lkeval.c
 *		a_uint	eval()		lkeval.c
 *		VOID	getid()		lklex.c
 *		VOID *	new()		lksym.c
 *		VOID	lkparea()	lkarea.c
 *		int	more()		lklex.c
 *		int	symeq()		lksym.c
 *
 *	side effects:
 *		A new head structure is created and linked to any
 *		existing linked head structure.  The head structure
 *		parameters of file handle, number of areas, number
 *		of global symbols, number of banks and number of
 *		merge modes are loaded into the structure.  The
 *		area, bank, symbol, and mode structure lists are created.
 *		The default area "_abs_" is created when the first
 *		head structure is created and an areax structure is
 *		created for every head structure called.
 */

/*
 * Create a new header entry.
 *
 * H n areas n global symbols n banks n modes
 *   |       |                |       |
 *   |       |                |       `----- G Lines
 *   |       |                `------------- B Lines
 *   |	     `------------------------------ hp->h_nsym
 *   `-------------------------------------- hp->h_narea
 *
 */
VOID
newhead()
{
	int i;
	char id[NCPS];
	struct head *thp;

	hp = (struct head *) new (sizeof(struct head));
	if (headp == NULL) {
		headp = hp;
	} else {
		thp = headp;
		while (thp->h_hp)
			thp = thp->h_hp;
		thp->h_hp = hp;
	}
	/*
	 * Initialize the header
	 */
	hp->h_lfile = cfp;		/* Set file pointer */
	hp->m_id = "";			/* No Module */
	/*
	 * Scan for Parameters	 
	 */
	while (more()) {
		i = (int) eval();
		getid(id, -1);
		/*
		 * Area pointer list
		 */
		if (symeq("areas", id, 1)) {
			hp->h_narea = i;
			if (i)
				hp->a_list = (struct areax **) new (i*sizeof(struct areax *));
		} else
		/*
		 * Symbol pointer list
		 */
		if (symeq("global", id, 1)) {
			hp->h_nsym = i;
			if (i)
				hp->s_list = (struct sym **) new (i*sizeof(struct sym *));
			skip(-1);
		} else
		/*
		 * Bank pointer list
		 */
		if (symeq("banks", id, 1)) {
			hp->h_nbank = i;
			if (i)
				hp->b_list = (struct bank **) new (i*sizeof(struct bank *));
		} else
		/*
		 * Mode pointer list
		 */
		if (symeq("modes", id, 1)) {
			hp->h_nmode = i;
			if (i)
				hp->m_list = (struct mode **) new (i*sizeof(struct mode *));
		}
	}
	/*
	 * Setup Absolute DEF linkage.
	 */
	lkparea(_abs_);
	ap->a_flag = A3_ABS;
	axp->a_addr = 0;
}


/*)Function	VOID	newmode()
 *
 *	The function newmode() creates / fills in a merge mode
 *	definition for this module.
 *
 *	The MODE structure contains the specification of one of the
 *	assemblers' relocation modes.  Each assembler must specify
 *	at least one relocation mode.  The relocation specification
 *	allows arbitrarily defined active bits and bit positions.
 *	The 32 element arrays are indexed from 0 to 31.
 *	Index 0 corresponds to bit 0, ..., and 31 corresponds to bit 31
 *	of a normal integer value.
 *
 *	The value an array element defines if the normal integer bit is
 *	active (bit <7> is set, 0x80) and what destination bit
 *	(bits <4:0>, 0 - 31) should be loaded with this normal
 *	integer bit.
 *
 *	The mode structure also contains a flag indicating if bit
 *	positioning is required, a mask word containing the active
 *	bits for merging, and an address paging mask.
 *
 *	local variables:
 *		int	index		bit index of mode definition
 *		int	n		counter
 *		struct mode *mp		pointer to a mode structure
 *
 *	global variables:
 *		head	*headp		The pointer to the first
 *				 	head structure of a linked list
 *		head	*hp		Pointer to the current
 *				 	head structure
 *		FILE *	stderr		standard library error handle
 *
 *	functions called:
 *		a_uint	eval()		lkexpr.c
 *		int	fprintf()	c_library
 *		VOID	lkexit()	lkmain.c
 *		int	more()		lklex.c
 *		char *	new()		lksym.c
 *
 *	side effects:
 *		The merge mode structure is created / updated with
 *		the definition values.
 */

VOID
newmode()
{
	int index, n;
	a_uint v;
	struct mode *mp;

	if (headp == NULL) {
		fprintf(stderr, "No header defined\n");
		lkexit(ER_FATAL);
	}
	/*
	 * Mode number
	 */
	n = (int) eval();
	if (n >= hp->h_nmode) {
		fprintf(stderr, "Header mode list overflow\n");
		lkexit(ER_FATAL);
	}
	/*
	 * Bit index
	 */
	index = (int) eval();
	if (index == 0) {
		mp = (struct mode *) new (sizeof(struct mode));
		hp->m_list[n] = mp;
		/*
		 * Initialize Mode
		 */
		for (n=0; n<32; n++) {
			mp->m_def[n] = n;
		}
	} else {
		mp = hp->m_list[n];
	}
	/*
	 * Load Bits
	 */
	while (more() && (index < 32)) {
		n = (int) eval();
		if (mp->m_def[index] != (n & 0x1F)) {
			mp->m_flag |= 1;
		}
		mp->m_def[index] = n;
		if (n & 0x80) {
			mp->m_dbits |= (((a_uint) 1) << (n & 0x1F));
			mp->m_sbits |= (((a_uint) 1) << index);
		}
		index += 1;
	}
	/*
	 * Set Missing Low Order Bits
	 */
	for (n=0; n<32; n++) {
		v = 1 << n;
		if (mp->m_sbits & v) {
			break;
		} else {
			mp->m_sbits |= v;
		}
	}
}


/*)Function	VOID	module()
 *
 *	The function module() copies the module name into
 *	the current head structure.
 *
 *	local variables:
 *		char	id[]		module id string
 *
 *	global variables:
 *		head	*headp		The pointer to the first
 *					head structure of a linked list
 *		head	*hp		Pointer to the current
 *					head structure
 *		int	lkerr		error flag
 *		FILE *	stderr		c_library
 *
 *	functions called:
 *		int	fprintf()	c_library
 *		VOID	getid()		lklex.c
 *		char *	strsto()	lksym.c
 *
 *	side effects:
 *		The module name is copied into the head structure.
 */

VOID
module()
{
	char id[NCPS];

	if (headp) {
		getid(id, -1);
		hp->m_id = strsto(id);
	} else {
		fprintf(stderr, "No header defined\n");
		lkerr++;
	}
}
