/*
 * calc.h - header file for the calc package
 * rob linwood (rcl211@nyu.edu)
 * see README for more information
 */

#ifndef _calc_h_
#define _calc_h_

#define CALC_SUCCESS 0      /* Indicates success */
#define CALC_NO_OP 1        /* No mathematical operator in expression */
#define CALC_BAD_OP 2       /* Unknown mathematical operator in expression */

int calc( char *expr, int *status );

#endif
