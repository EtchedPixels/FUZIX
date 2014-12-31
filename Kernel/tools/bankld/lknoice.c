/* lknoice.c */

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
 *
 *
 * Extensions to produce NoICE debug files
 *
 * 31-Oct-1997 by John Hartman
 * 30-Jan-98 JLH add page to DefineNoICE for 8051
 *  2-Feb-98 JLH Allow optional .nest on local vars - C scoping rules...
 * 27-May-01 ARB Updated for ASxxxx V4
 */

#include "aslink.h"


#if NOICE

/*Module        lknoice.c
 *
 *      The module lknoice.c contains the functions
 *      required to create a NoICE debug file.
 *
 *      lknoice.c contains the following functions:
 *              VOID    NoICEfopen()
 *              VOID    NoICEmagic()
 *              VOID    DefineNoICE()
 *              VOID    DefineGlobal()
 *              VOID    DefineScoped()
 *              VOID    DefineFile()
 *              VOID    DefineFunction()
 *              VOID    DefineStaticFunction()
 *              VOID    DefineEndFunction()
 *              VOID    DefineLine()
 *              VOID    PagedAddress()
 *
 *      lknoice.c contains these local variables:
 *              struct noicebn *noicebnp        pointer to linked structure of
 *                                              ';!FILE' specifications
 *              char currentFile[]              file being processed
 *              char currentFunction[]          function being processed
 */

struct  noicefn {
        struct  noicefn *n_np;  /* noicefn link */
        char *  n_id;           /* file name */
};

static struct noicefn *noicefnp = NULL;

static char currentFile[NCPS];
static char currentFunction[NCPS];


/*)Function     VOID    NoICEfopen()
 *
 *      The function NoICEfopen() opens the NoICE output file
 *      and sets the map flag, mflag, to create a map file.
 *      NoICE processing is performed during map generation.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              int     jflag           NoICE Debug flag
 *              FILE *  jfp             NoICE Debug File handle
 *              struct lfile *linkp     Pointer to the Linker output file name
 *              int     mflag           Map output flag
 *
 *      functions called:
 *              FILE *  afile()         lkmain.c
 *              VOID    lkexit()        lkmain.c
 *
 *      side effects:
 *              The NoICE output file is opened.
 *              Failure to open the file will
 *              terminate the linker.
 */

VOID NoICEfopen(void)
{
        if (jflag) {
                jfp = afile(linkp->f_idp, "noi", 1);
                if (jfp == NULL) {
                        lkexit(1);
                }
                mflag = 1;
        }
}


/*)Function     VOID    NoICEmagic()
 *
 *      The function NoICEmagic() passes any "magic Comments"
 *      to the NoICE output file.  Magic comments are those
 *      beginning with ";!".  Also a linked list of file names
 *      specified in ";!FILE" magic comments is created.  These
 *      file names are used to verify that symbols in the
 *      ASxxxx .rel files of the form str1.str2 are NoICE symbols.
 *
 *      local variables:
 *              char    id[]            id string
 *              struct noicefn * np     pointer to new structure
 *              char *  p1              temporary string pointer
 *              char *  p2              temporary string pointer
 *              struct noicefn * tnp    temporary pointer to noicefn structure
 *
 *      global variables:
 *              char *  ip              position into the current
 *                                      input text line
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              VOID    getid()         lklex.c
 *              VOID *  new()           lksym.c
 *              int     fprintf()       c_library
 *              char *  strrchr()       c_library
 *              char *  strsto()        lksym.c
 *              int     symeq()         lksym.c
 *
 *      side effects:
 *              The NoICE "magic comments" are passed
 *              to the output file.  A list of assembler
 *              file names is created.
 */

VOID NoICEmagic(void)
{
        char id[NCPS];
        char *p1, *p2;
        struct noicefn *np, *tnp;

        /*
         * Pass any "magic comments" to NoICE output
         */
        if ((ip[0] == ';') && (ip[1] == '!')) {
                if (jfp) {
                        fprintf(jfp, "%s\n", &ip[2]);
                }
                if (pass == 0) {
                        getid(id, -1);
                        if (symeq(id, ";!FILE", 1)) {
                                getid(id, -1);
                                /*
                                 * The name starts after the last
                                 * '/' (Unices) or
                                 * ':' or '\' (DOS)
                                 *
                                 * and ends at the last
                                 * separator 'FSEPX'
                                 */
                                p1 = id;
                                if ((p2 = strrchr(p1,  '\\')) != NULL)  p1 = ++p2;
                                if ((p2 = strrchr(p1,   '/')) != NULL)  p1 = ++p2;
                                if ((p2 = strrchr(p1,   ':')) != NULL)  p1 = ++p2;
                                if ((p2 = strrchr(p1, FSEPX)) != NULL) *p2 = 0;

                                np = (struct noicefn *) new (sizeof(struct noicefn));
                                if (noicefnp == NULL) {
                                        noicefnp = np;
                                } else {
                                        tnp = noicefnp;
                                        while (tnp->n_np)
                                                tnp = tnp->n_np;
                                        tnp->n_np = np;
                                }
                                np->n_id = strsto(p1);
                        }
                }
        }
}


/*)Function     VOID    DefineNoICE()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineNoICE() processes the symbols into
 *      NoICE commands for inclusion in the NoICE output file.
 *
 *      The function is called from lstarea in lklist.c
 *      for each symbol.
 *
 *      local variables:
 *              int     j               parsed argument count
 *              int     k               parsed argument count
 *              int     level           function level
 *              char    token1[]        parsed string
 *              char    token2[]        parsed string
 *              char    token2[]        parsed string
 *              char    sep1            parsed character
 *              char    sep2            parsed character
 *              struct noicefn * tnp    temporary pointer to noicefn structure
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              VOID    DefineFile()            lknoice.c
 *              VOID    DefineFunction()        lknoice.c
 *              VOID    DefineStaticFunction()  lknoice.c
 *              VOID    DefineEndFunction()     lknoice.c
 *              VOID    DefineScoped()          lknoice.c
 *              VOID    DefineLine()            lknoice.c
 *              VOID    DefineGlobal()          lknoice.c
 *              VOID    PagedAddress()          lknoice.c
 *              int     sprintf()               c_library
 *              int     sscanf()                c_library
 *              int     symeq()                 lksym.c
 *
 *      side effects:
 *              NoICE debug commands are placed
 *              into the output file.
 */

void DefineNoICE( char *name, a_uint value, struct bank *yp )
{
        char token1[NCPS];                      /* parse for file.function.symbol */
        char token2[NCPS];
        char token3[NCPS];
        char sep1, sep2;
        int  j, k, level;
        struct noicefn *np;

        /* no output if file is not open */
        if (jfp == NULL) return;

        j = sscanf( name, "%[^.]%c%[^.]%c%s", token1, &sep1, token2, &sep2, token3 );
        if (j > 1) {
                /* verify that first token is a file name */
                k = 1;
                np = noicefnp;
                while (np != NULL) {
                        if (symeq(token1, np->n_id, 1)) {
                                k = j;
                                break;
                        }
                        np = np->n_np;
                }
                j = k;
        }

        switch (j)
        {
                /* file.function.symbol, or file.function..SPECIAL */
                case 5:
                        DefineFile( token1, 0, NULL );
                        if (token3[0] == '.')
                        {
                                if (symeq( token3, ".FN", 1 ) != 0)
                                {
                                        /* Global function */
                                        DefineFunction( token2, value, yp );
                                }
                                else if (symeq( token3, ".SFN", 1 ) != 0)
                                {
                                        /* Static (file-scope) function */
                                        DefineStaticFunction( token2, value, yp );
                                }
                                else if (symeq( token3, ".EFN", 1 ) != 0)
                                {
                                        /* End of function */
                                        DefineEndFunction( value, yp );
                                }
                        }
                        else
                        {
                                /* Function-scope var. */
                                DefineFunction( token2, 0, NULL );

                                /* Look for optional level integer */
                                j = sscanf( token3, "%[^.]%c%u", token1, &sep1, &level );
                                if ((j == 3) && (level != 0))
                                {
                                        sprintf( &token1[ strlen(token1) ], "_%u", level );
                                }
                                DefineScoped( token1, value, yp );
                        }
                        break;

                /* either file.symbol or file.line# */
                case 3:
                        DefineFile( token1, 0, NULL );
                        if ((token2[0] >= '0') && (token2[0] <= '9'))
                        {
                                /* Line number */
                                DefineLine( token2, value, yp );
                        }
                        else
                        {
                                /* File-scope symbol.  (Kill any function) */
                                DefineEndFunction( 0, NULL );
                                DefineScoped( token2, value, yp );
                        }
                        break;

                /* NoICE file.func. is illegal */
                case 4:

                /* NoICE symbol. is illegal */
                case 2:

                /* just a symbol */
                case 1:

                /* NoICE .symbol is illegal */
                case 0:
                default:
                        DefineGlobal( name, value, yp );
                        break;
        }
}


/*)Function     VOID    DefineGlobal()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineGlobal() places a DEF statement
 *      in the .noi debug file for the global symbol.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *
 *      side effects:
 *              A global symbol definition is
 *              placed in the .noi debug file.
 */

void DefineGlobal( char *name, a_uint value, struct bank *yp )
{
        fprintf( jfp, "DEF %s ", name );
        PagedAddress( value, yp );
}


/*)Function     VOID    DefineScoped()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineScoped() places a DEFS statement
 *      in the .noi debug file for the scoped symbol.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *
 *      side effects:
 *              A scoped symbol definition is
 *              placed in the .noi debug file.
 */

void DefineScoped( char *name, a_uint value, struct bank *yp )
{
        fprintf( jfp, "DEFS %s ", name );
        PagedAddress( value, yp );
}


/*)Function     VOID    DefineFile()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineFile() places a FILE statement
 *      in the .noi debug file for the processed file.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *              char *  strcpy()        c_library
 *              int     symeq()         lksym.c
 *
 *      side effects:
 *              A file name definition is
 *              placed in the .noi debug file.
 */

void DefineFile( char *name, a_uint value, struct bank *yp )
{
        if (symeq( name, currentFile, 1 ) == 0)
        {
                strcpy( currentFile, name );
                if (value != 0)
                {
                        fprintf( jfp, "FILE %s ", name );
                        PagedAddress( value, yp );
                }
                else
                {
                        fprintf( jfp, "FILE %s\n", name );
                }
        }
}


/*)Function     VOID    DefineFunction()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineFunction() places a FUNC statement
 *      in the .noi debug file for the processed symbol.  If
 *      a vaulue is present then a preceeding DEF statement is
 *      also placed in the .noi debug file.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *              char *  strcpy()        c_library
 *              int     symeq()         lksym.c
 *
 *      side effects:
 *              A function definition is
 *              placed in the .noi debug file.
 */

void DefineFunction( char *name, a_uint value, struct bank *yp )
{
        if (symeq( name, currentFunction, 1 ) == 0)
        {
                strcpy( currentFunction, name );
                if (value != 0)
                {
                        fprintf( jfp, "DEF %s ", name );
                        PagedAddress( value, yp );
                        fprintf( jfp, "FUNC %s ", name );
                        PagedAddress( value, yp );
                }
                else
                {
                        fprintf( jfp, "FUNC %s\n", name );
                }
        }
}


/*)Function     VOID    DefineStaticFunction()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineStaticFunction() places a SFUNC statement
 *      in the .noi debug file for the processed file.  If
 *      a value is present then a preceeding DEFS statement is
 *      also placed in the .noi debug file.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *              char *  strcpy()        c_library
 *              int     symeq()         lksym.c
 *
 *      side effects:
 *              A static function definition is
 *              placed in the .noi debug file.
 */

void DefineStaticFunction( char *name, a_uint value, struct bank *yp )
{
        if (symeq( name, currentFunction, 1 ) == 0)
        {
                strcpy( currentFunction, name );
                if (value != 0)
                {
                        fprintf( jfp, "DEFS %s ", name );
                        PagedAddress( value, yp );
                        fprintf( jfp, "SFUNC %s ", name );
                        PagedAddress( value, yp );
                }
                else
                {
                        fprintf( jfp, "SFUNC %s\n", name );
                }
        }
}


/*)Function     VOID    DefineEndFunction()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineEndFunction() places an ENDF statement
 *      in the .noi debug file for the processed file.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *              char *  strcpy()        c_library
 *              int     symeq()         lksym.c
 *
 *      side effects:
 *              An end function definition is
 *              placed in the .noi debug file.
 */

void DefineEndFunction( a_uint value, struct bank *yp )
{
        if (currentFunction[0] != 0)
        {
                if (value != 0)
                {
                        fprintf( jfp, "ENDF " );
                        PagedAddress( value, yp );
                }
                else
                {
                        fprintf( jfp, "ENDF\n" );
                }

                currentFunction[0] = 0;
        }
}


/*)Function     VOID    DefineLine()
 *
 *              char *          name    pointer to the symbol string
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function DefineLine() places a LINE statement
 *      in the .noi debug file for the processed file.
 *
 *      local variables:
 *              int     indigit         converted digit
 *              int     lineNumber      converted line number
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *              VOID    PagedAddress()  lknoice.c
 *              int     digit()         lkeval.c
 *
 *      side effects:
 *              A Line definition is
 *              placed in the .noi debug file.
 */

void DefineLine( char *lineString, a_uint value, struct bank *yp )
{
        int indigit, lineNumber;

        lineNumber = 0;
        while( (indigit=digit( *lineString++, 10 )) >= 0)
        {
                lineNumber = 10*lineNumber + indigit;
        }
        fprintf( jfp, "LINE %u ", lineNumber );
        PagedAddress( value, yp );
}


/*)Function     VOID    PagedAddress()
 *
 *              a_uint          value   value of symbol
 *              struct bank *   yp      pointer to associated bank
 *
 *      The function PagedAddress() places the value
 *      in the .noi debug file for the processed value.
 *      If the current bank is "mapped" then the page
 *      number preceeds the value as xx:.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             NoICE Debug File handle
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              A value is appended to the current
 *              line placed in the .noi debug file.
 */

void PagedAddress( a_uint value, struct bank *yp )
{
        if (yp->b_flag & B_MAP) {
                fprintf( jfp, "%X:0x%X\n", yp->b_map, value );
        } else {
                fprintf( jfp, "0x%X\n", value );
        }
}

#endif
