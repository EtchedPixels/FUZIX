/* lkmain.c */

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

/*)Module       lkmain.c
 *
 *      The module lkmain.c contains the functions which
 *      (1) input the linker options, parameters, and specifications
 *      (2) perform a two pass link
 *      (3) produce the appropriate linked data output and/or
 *          link map file and/or relocated listing files.
 *
 *      lkmain.c contains the following functions:
 *              FILE *  afile()
 *              VOID    bassav()
 *              VOID    gblsav()
 *              int     intsiz()
 *              VOID    link_main()
 *              VOID    lkexit()
 *              int     fndext()
 *              int     fndidx()
 *              int     main()
 *              VOID    map()
 *              int     parse()
 *              VOID    doparse()
 *              VOID    setgbl()
 *              VOID    usage()
 *
 *      lkmain.c contains the following local variables:
 *              char *  usetext[]       array of pointers to the
 *                                      command option tect lines
 *
 */

/* sdld 8051 specific */
/*JCF:  Creates some of the default areas so they are allocated in the right order.*/
void Areas51 (void)
{
        char * rel[] = {
                "XH",
                "H 7 areas 0 global symbols",
                "A _CODE size 0 flags 0",               /*Each .rel has one, so...*/
                "A REG_BANK_0 size 0 flags 4",  /*Register banks are overlayable*/
                "A REG_BANK_1 size 0 flags 4",
                "A REG_BANK_2 size 0 flags 4",
                "A REG_BANK_3 size 0 flags 4",
                "A BSEG size 0 flags 80",               /*BSEG must be just before BITS*/
                "A BSEG_BYTES size 0 flags 0",  /*Size will be obtained from BSEG in lnkarea()*/
                ""
        };

        char * rel2[] = {
                "XH",
                "H C areas 0 global symbols",
                "A _CODE size 0 flags 0",               /*Each .rel has one, so...*/
                "A REG_BANK_0 size 0 flags 4",  /*Register banks are overlayable*/
                "A REG_BANK_1 size 0 flags 4",
                "A REG_BANK_2 size 0 flags 4",
                "A REG_BANK_3 size 0 flags 4",
                "A BSEG size 0 flags 80",               /*BSEG must be just before BITS*/
                "A BSEG_BYTES size 0 flags 0",  /*Size will be obtained from BSEG in lnkarea()*/
                "A BIT_BANK size 0 flags 4",    /*Bit register bank is overlayable*/
                "A DSEG size 0 flags 0",
                "A OSEG size 0 flags 4",
                "A ISEG size 0 flags 0",
                "A SSEG size 0 flags 4",
                ""
        };
        int j;
        struct sym * sp;

        if (packflag) {
                for (j = 0; rel2[j][0] != 0; j++) {
                        ip = rel2[j];
                        link_main();
                }
        }
        else {
                for (j = 0; rel[j][0] != 0; j++) {
                        ip = rel[j];
                        link_main();
                }
        }

        /*Set the start address of the default areas:*/
        for (ap = areap; ap; ap = ap->a_ap) {
                /**/ if (!strcmp(ap->a_id, "REG_BANK_0")) { ap->a_addr = 0x00; ap->a_bset = 1; }
                else if (!strcmp(ap->a_id, "REG_BANK_1")) { ap->a_addr = 0x08; ap->a_bset = 1; }
                else if (!strcmp(ap->a_id, "REG_BANK_2")) { ap->a_addr = 0x10; ap->a_bset = 1; }
                else if (!strcmp(ap->a_id, "REG_BANK_3")) { ap->a_addr = 0x18; ap->a_bset = 1; }
                else if (!strcmp(ap->a_id, "BSEG_BYTES")) { ap->a_addr = 0x20; ap->a_bset = 1; }
                else if (TARGET_IS_8051 && !strcmp(ap->a_id, "SSEG")) {
                        if (stacksize) ap->a_axp->a_size = stacksize;
                }
        }

        sp = lkpsym("l_IRAM", 1);
        sp->s_addr = ((iram_size>0) && (iram_size<=0x100)) ? iram_size : 0x0100;
        sp->s_axp = NULL;
        sp->s_type |= S_DEF;
}
/* end sdld 8051 specific */

/*)Function     int     main(argc,argv)
 *
 *              int     argc            number of command line arguments + 1
 *              char *  argv[]          array of pointers to the command line
 *                                      arguments
 *
 *      The function main() evaluates the command line arguments to
 *      determine if the linker parameters are to input through 'stdin'
 *      or read from a command file.  The functions nxtline() and parse()
 *      are to input and evaluate the linker parameters.  The linking process
 *      proceeds by making the first pass through each .rel file in the order
 *      presented to the linker.  At the end of the first pass the setarea(),
 *      lnkarea(), setgbl(), and symdef() functions are called to evaluate
 *      the base address terms, link all areas, define global variables,
 *      and look for undefined symbols.  Following these routines a linker
 *      map file may be produced and the linker output files may be opened.
 *      The second pass through the .rel files will output the linked data
 *      in one of the supported formats.
 *
 *      local variables:
 *              int     c               character from argument string
 *              int     i               loop counter
 *              int     j               loop counter
 *              int     k               loop counter
 *
 *      global variables:
 *                                      text line in ib[]
 *              lfile   *cfp            The pointer *cfp points to the
 *                                      current lfile structure
 *              char    ctype[]         array of character types, one per
 *                                      ASCII character
 *              lfile   *filep          The pointer *filep points to the
 *                                      beginning of a linked list of
 *                                      lfile structures.
 *              head    *hp             Pointer to the current
 *                                      head structure
 *              char    ib[NINPUT]      .rel file text line
 *              char    *ip             pointer into the .rel file
 *              lfile   *linkp          pointer to first lfile structure
 *                                      containing an input .rel file
 *                                      specification
 *              int     lkerr           error flag
 *              int     oflag           Output file type flag
 *              int     objflg          Linked file/library output object flag
 *              int     pass            linker pass number
 *              int     pflag           print linker command file flag
 *              int     radix           current number conversion radix
 *              FILE    *sfp            The file handle sfp points to the
 *                                      currently open file
 *              lfile   *startp         aslink startup file structure
 *              FILE *  stdout          c_library
 *
 *      functions called:
 *              VOID    chkbank()       lkbank.c
 *              int     fclose()        c_library
 *              int     fprintf()       c_library
 *              VOID    library()       lklibr.c
 *              VOID    link_main()     lkmain.c
 *              VOID    lkexit()        lkmain.c
 *              VOID    lkfopen()       lkbank.c
 *              VOID    lnkarea()       lkarea.c
 *              VOID    map()           lkmain.c
 *              VOID    new()           lksym.c
 *              int     nxtline()       lklex.c
 *              int     parse()         lkmain.c
 *              VOID    reloc()         lkreloc.c
 *              VOID    search()        lklibr.c
 *              VOID    setarea()       lkarea.c
 *              VOID    setbank()       lkbank.c
 *              VOID    setgbl()        lkmain.c
 *              char *  sprintf()       c_library
 *              VOID    symdef()        lksym.c
 *              VOID    usage()         lkmain.c
 *              int     fndidx()        lkmain.c
 *
 *      side effects:
 *              Completion of main() completes the linking process
 *              and may produce a map file (.map) and/or a linked
 *              data files (.ihx or .s19) and/or one or more
 *              relocated listing files (.rst).
 */

int
main(int argc, char *argv[])
{
        int c, i, j, k;

        if (intsiz() < 4) {
                fprintf(stderr, "?ASlink-Error-Size of INT32 is not 32 bits or larger.\n\n");
                exit(ER_FATAL);
        }

        /* sdas specific */
        /* sdas initialization */
        sdld_init(argv[0]);

        /* use these defaults for parsing the .lnk script */
        a_bytes = 4;
        a_mask = 0xFFFFFFFF;
        s_mask = 0x80000000;
        v_mask = 0x7FFFFFFF;
        /* end sdas specific */

        if (!is_sdld())
                fprintf(stdout, "\n");

        startp = (struct lfile *) new (sizeof (struct lfile));
        startp->f_idp = "";

        pflag = 1;

        for(i=1; i<argc; i++) {
                ip = ib;
                if(argv[i][0] == '-') {
                        j = i;
                        k = 1;
                        while((c = argv[j][k]) != '\0') {
                                ip = ib;
                                sprintf(ip, "-%c", c);
                                switch(c) {

                                /*
                                 * Options with arguments
                                 */
                                case 'b':
                                case 'B':

                                case 'g':
                                case 'G':

                                case 'k':
                                case 'K':

                                case 'l':
                                case 'L':

                                case 'f':
                                case 'F':

                                case 'I':
                                case 'X':
                                case 'C':
                                case 'S':
                                        strcat(ip, " ");
                                        if (i < argc - 1)
                                                strcat(ip, argv[++i]);
					else
                                                strcpy(ip, "");
                                        break;
                                /*
                                 * Preprocess these commands
                                 */
                                case 'n':
                                case 'N':
                                        pflag = 0;
                                        break;

                                case 'p':
                                case 'P':
                                        pflag = 1;
                                        break;

                                /*
                                 * Options without arguments
                                 */
                                default:
                                        break;
                                }
                                if(pflag)
                                        fprintf(stdout, "ASlink >> %s\n", ip);
                                parse();
                                k++;
                        }
                } else {
                        strcpy(ip, argv[i]);
                        if(pflag)
                                fprintf(stdout, "ASlink >> %s\n", ip);
                        parse();
                }
        }

        if (linkp == NULL)
                usage(ER_FATAL);

        /*
         * If no input file is specified
         * then assume a single file with
         * the same name as the output file.
         */
        if (lfp == linkp) {
                lfp->f_flp = (struct lfile *) new (sizeof (struct lfile));
                lfp = lfp->f_flp;
                lfp->f_idp = strsto(linkp->f_idp);
                lfp->f_idx = fndidx(linkp->f_idp);
                lfp->f_obj = objflg;
                lfp->f_type = F_REL;
        }

        syminit();

#if SDCDB
        /*
         * Open SDCC Debug output file
         */
        SDCDBfopen();
#endif

        for (pass=0; pass<2; ++pass) {
                cfp = NULL;
                sfp = NULL;
                filep = linkp->f_flp;
                hp = NULL;
                radix = 10;

                /* sdld specific */
                if (TARGET_IS_8051)
                        Areas51(); /*JCF: Create the default 8051 areas in the right order*/
                /* end sdld specific */

                while (nxtline()) {
                        ip = ib;
                        link_main();
                }
                if (pass == 0) {
                        /*
                         * Search libraries for global symbols
                         */
                        search();

                        /* sdas specific */
                        /* use these defaults for parsing the .lk script */
                        a_bytes = 4;
                        a_mask = 0xFFFFFFFF;
                        s_mask = 0x80000000;
                        v_mask = 0x7FFFFFFF;
                        /* end sdas specific */

                        /*
                         * Set area base addresses.
                         */
                        setarea();
                        /*
                         * Set bank base addresses.
                         */
                        setbank();
                        /*
                         * Link all area addresses.
                         */
                        if (!packflag)
                                lnkarea();
                        else {
                                /* sdld 8051 specific */
                                lnkarea2();
                                /* end sdld 8051 specific */
                        }
                        /*
                         * Check bank size limits.
                         */
                        chkbank(stderr);
                        /*
                         * Process global definitions.
                         */
                        setgbl();
                        /*
                         * Check for undefined globals.
                         */
                        symdef(stderr);
#if NOICE
                        /*
                         * Open NoICE output file
                         */
                        NoICEfopen();
#endif
                        /*
                         * Output Link Map.
                         */
                        map();

                        /* sdld specific */
                        if (sflag) {    /*JCF: memory usage summary output*/
                                if (!packflag) {
                                        if (summary(areap)) lkexit(1);
                                }
                                else {
                                        /* sdld 8051 specific */
                                        if (summary2(areap)) lkexit(1);
                                        /* end sdld 8051 specific */
                                }
                        }

                        if ((iram_size) && (!packflag))
                                iramcheck();
                        /* end sdld specific */

                        /*
                         * Open output file(s)
                         */
                        lkfopen();
                } else {
                        /*
                         * Link in library files
                         */
                        library();
                        /*
                         * Complete Processing
                         */
                        reloc('E');
                }
        }
        if (TARGET_IS_8051) {
                //JCF:
                CreateAOMF51();
        }

        lkexit(lkerr ? ER_ERROR : ER_NONE);
        return(0);
}

/*)Function     int     intsiz()
 *
 *      The function intsiz() returns the size of INT32
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              none
 *
 *      side effects:
 *              none
 */

int
intsiz()
{
        return(sizeof(a_uint));
}

/*)Function     VOID    lkexit(i)
 *
 *                      int     i       exit code
 *
 *      The function lkexit() explicitly closes all open
 *      files and then terminates the program.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  jfp             file handle for .noi
 *              FILE *  mfp             file handle for .map
 *              FILE *  rfp             file hanlde for .rst
 *              FILE *  sfp             file handle for .rel
 *              FILE *  tfp             file handle for .lst
 *
 *      functions called:
 *              int     fclose()        c_library
 *              VOID    exit()          c_library
 *              VOID    lkfclose()      lkbank.c
 *
 *      side effects:
 *              All files closed. Program terminates.
 */

VOID
lkexit(i)
int i;
{
        lkfclose();
#if NOICE
        if (jfp != NULL) fclose(jfp);
#endif
        if (mfp != NULL) fclose(mfp);
        if (rfp != NULL) fclose(rfp);
        if (sfp != NULL) { if (sfp != stdin) fclose(sfp); }
        if (tfp != NULL) fclose(tfp);
#if SDCDB
        if (yfp != NULL) fclose(yfp);
#endif
        exit(i);
}

/*)Function     link_main()
 *
 *      The function link_main() evaluates the directives for each line of
 *      text read from the .rel file(s).  The valid directives processed
 *      are:
 *              X, D, Q, H, M, A, S, T, R, and P.
 *
 *      local variables:
 *              int     c               first non blank character of a line
 *
 *      global variables:
 *              head    *headp          The pointer to the first
 *                                      head structure of a linked list
 *              head    *hp             Pointer to the current
 *                                      head structure
 *              int     a_bytes         T Line address bytes
 *              int     hilo            Byte ordering
 *              int     pass            linker pass number
 *              int     radix           current number conversion radix
 *
 *      functions called:
 *              char    endline()       lklex.c
 *              VOID    module()        lkhead.c
 *              VOID    newarea()       lkarea.c
 *              VOID    newhead()       lkhead.c
 *              sym *   newsym()        lksym.c
 *              VOID    NoICEmagic()    lknoice.c
 *              VOID    reloc()         lkreloc.c
 *
 *      side effects:
 *              Head, area, and symbol structures are created and
 *              the radix is set as the .rel file(s) are read.
 */

VOID
link_main()
{
        char c;

        if ((c=endline()) == 0) { return; }
        switch (c) {

        /* sdld specific */
        case 'O': /* For some important sdcc options */
                if (is_sdld() && pass == 0) {
                        if (NULL == optsdcc) {
                                optsdcc = strsto(&ip[1]);
                                optsdcc_module = hp->m_id;
                        }
                        else {
                                if (strcmp(optsdcc, &ip[1]) != 0) {
                                        fprintf(stderr,
                                                "?ASlink-Warning-Conflicting sdcc options:\n"
                                                "   \"%s\" in module \"%s\" and\n"
                                                "   \"%s\" in module \"%s\".\n",
                                                optsdcc, optsdcc_module, &ip[1], hp->m_id);
                                        lkerr++;
                                }
                        }
                }
                break;
        /* end sdld specific */

        case 'X':
        case 'D':
        case 'Q':
                ASxxxx_VERSION = 3;
                a_bytes = 2;    /* use default if unspecified */
                hilo = 0;       /* use default if unspecified */
                if (c == 'X') { radix = 16; } else
                if (c == 'D') { radix = 10; } else
                if (c == 'Q') { radix = 8;  }

                while ((c = get()) != 0) {
                        switch(c) {
                        case 'H':
                                hilo = 1;
                                break;

                        case 'L':
                                hilo = 0;
                                break;

                        case '2':
                                a_bytes = 2;
                                break;

                        case '3':
                                a_bytes = 3;
                                break;

                        case '4':
                                a_bytes = 4;
                                break;

                        default:
                                break;
                        }
                }
#ifdef  LONGINT
                switch(a_bytes) {
                default:
                        a_bytes = 2;
                case 2:
                        a_mask = 0x0000FFFFl;
                        s_mask = 0x00008000l;
                        v_mask = 0x00007FFFl;
                        break;

                case 3:
                        a_mask = 0x00FFFFFFl;
                        s_mask = 0x00800000l;
                        v_mask = 0x007FFFFFl;
                        break;

                case 4:
                        a_mask = 0xFFFFFFFFl;
                        s_mask = 0x80000000l;
                        v_mask = 0x7FFFFFFFl;
                        break;
                }
#else
                switch(a_bytes) {
                default:
                        a_bytes = 2;
                case 2:
                        a_mask = 0x0000FFFF;
                        s_mask = 0x00008000;
                        v_mask = 0x00007FFF;
                        break;

                case 3:
                        a_mask = 0x00FFFFFF;
                        s_mask = 0x00800000;
                        v_mask = 0x007FFFFF;
                        break;

                case 4:
                        a_mask = 0xFFFFFFFF;
                        s_mask = 0x80000000;
                        v_mask = 0x7FFFFFFF;
                        break;
                }
#endif
                break;

        case 'H':
                if (pass == 0) {
                        newhead();
                } else {
                        if (hp == 0) {
                                hp = headp;
                        } else {
                                hp = hp->h_hp;
                        }
                }
                sdp.s_area = NULL;
                sdp.s_areax = NULL;
                sdp.s_addr = 0;
                break;

        case 'M':
                if (pass == 0)
                        module();
                break;

        case 'A':
                if (pass == 0)
                        newarea();
                if (sdp.s_area == NULL) {
                        sdp.s_area = areap;
                        sdp.s_areax = areap->a_axp;
                        sdp.s_addr = 0;
                }
                break;

        case 'S':
                if (pass == 0)
                        newsym();
                break;

        case 'T':
        case 'R':
        case 'P':
                if (pass == 0)
                        break;
                reloc(c);
                break;

#if NOICE
        case ';':
                unget(c);
                NoICEmagic();
                break;
#endif

        default:
                break;
        }
}

/*)Function     VOID    map()
 *
 *      The function map() opens the output map file and calls the various
 *      routines to
 *      (1) output the variables in each area,
 *      (2) list the files processed with module names,
 *      (3) list the libraries file processed,
 *      (4) list base address definitions,
 *      (5) list global variable definitions, and
 *      (6) list any undefined variables.
 *
 *      local variables:
 *              int     i               counter
 *              head *  hdp             pointer to head structure
 *              lbfile *lbfh            pointer to library file structure
 *
 *      global variables:
 *              area    *ap             Pointer to the current
 *                                      area structure
 *              area    *areap          The pointer to the first
 *                                      area structure of a linked list
 *              base    *basep          The pointer to the first
 *                                      base structure
 *              base    *bsp            Pointer to the current
 *                                      base structure
 *              lfile   *filep          The pointer *filep points to the
 *                                      beginning of a linked list of
 *                                      lfile structures.
 *              globl   *globlp         The pointer to the first
 *                                      globl structure
 *              globl   *gsp            Pointer to the current
 *                                      globl structure
 *              head    *headp          The pointer to the first
 *                                      head structure of a linked list
 *              lbfile  *lbfhead        The pointer to the first
 *                                      lbfile structure of a linked list
 *              lfile   *linkp          pointer to first lfile structure
 *                                      containing an input REL file
 *                                      specification
 *              int     lop             current line number on page
 *              int     mflag           Map output flag
 *              FILE    *mfp            Map output file handle
 *              int     page            current page number
 *
 *      functions called:
 *              FILE *  afile()         lkmain.c
 *              int     fprintf()       c_library
 *              VOID    lkexit()        lkmain.c
 *              VOID    lstarea()       lklist.c
 *              VOID    newpag()        lklist.c
 *              VOID    chkbank()       lkbank.c
 *              VOID    symdef()        lksym.c
 *
 *      side effects:
 *              The map file is created.
 */

VOID
map(void)
{
        int i;
        struct head *hdp;
        struct lbfile *lbfh;

        if (mflag == 0) return;

        /*
         * Open Map File
         */
        mfp = afile(linkp->f_idp, "map", 1);
        if (mfp == NULL) {
                lkexit(ER_FATAL);
        }

        /*
         * Output Map Bank/Area Lists
         */
        page = 0;
        lop  = NLPP;
        for (bp = bankp; bp != NULL; bp = bp->b_bp) {
                for (ap = areap; ap != NULL; ap = ap->a_ap) {
                        if (ap->a_bp == bp)
                                lstarea(ap, bp);
                }
        }

        /*
         * List Linked Files
         */
        newpag(mfp);
        fprintf(mfp, "\nFiles Linked                              [ module(s) ]\n\n");
        hdp = headp;
        filep = linkp->f_flp;
        while (filep) {
                if (strlen (filep->f_idp) > 40)
                        fprintf(mfp, "%s\n%40s  [ ", filep->f_idp, "");
                else
                        fprintf(mfp, "%-40.40s  [ ", filep->f_idp);
                i = 0;
                while ((hdp != NULL) && (hdp->h_lfile == filep)) {
                        if (i)
                                fprintf(mfp, ",\n%44s", "");
                        fprintf(mfp, "%-.32s", hdp->m_id);
                        hdp = hdp->h_hp;
                        i++;
                }
                fprintf(mfp, " ]\n");
                filep = filep->f_flp;
        }
        fprintf(mfp, "\n");
        /*
         * List Linked Libraries
         */
        if (lbfhead != NULL) {
                fprintf(mfp, "\nLibraries Linked                          [ object file ]\n\n");
                for (lbfh=lbfhead; lbfh; lbfh=lbfh->next) {
                        if (strlen (lbfh->libspc) > 40)
                                fprintf(mfp, "%s\n%40s  [ %-.32s ]\n",
                                        lbfh->libspc, "", lbfh->relfil);
                        else
                                fprintf(mfp, "%-40.40s  [ %-.32s ]\n",
                                        lbfh->libspc, lbfh->relfil);
                }
                fprintf(mfp, "\n");
        }
        /*
         * List Base Address Definitions
         */
        if (basep) {
                newpag(mfp);
                fprintf(mfp, "\nUser Base Address Definitions\n\n");
                bsp = basep;
                while (bsp) {
                        fprintf(mfp, "%s\n", bsp->b_strp);
                        bsp = bsp->b_base;
                }
        }
        /*
         * List Global Definitions
         */
        if (globlp) {
                newpag(mfp);
                fprintf(mfp, "\nUser Global Definitions\n\n");
                gsp = globlp;
                while (gsp) {
                        fprintf(mfp, "%s\n", gsp->g_strp);
                        gsp = gsp->g_globl;
                }
        }
        fprintf(mfp, "\n\f");
        chkbank(mfp);
        symdef(mfp);
}

/*)Function     int     parse()
 *
 *      The function parse() evaluates all command line or file input
 *      linker directives and updates the appropriate variables.
 *
 *      local variables:
 *              int     c               character value
 *              int     sv_type         save type of processing
 *              char    fid[]           file id string
 *
 *      global variables:
 *              char    ctype[]         array of character types, one per
 *                                      ASCII character
 *              lfile   *lfp            pointer to current lfile structure
 *                                      being processed by parse()
 *              lfile   *linkp          pointer to first lfile structure
 *                                      containing an input REL file
 *                                      specification
 *              int     mflag           Map output flag
 *              int     oflag           Output file type flag
 *              int     objflg          Linked file/library output object flag
 *              int     pflag           print linker command file flag
 *              FILE *  stderr          c_library
 *              int     uflag           Relocated listing flag
 *              int     xflag           Map file radix type flag
 *              int     wflag           Wide listing format
 *              int     zflag           Disable symbol case sensitivity
 *
 *      Functions called:
 *              VOID    addlib()        lklibr.c
 *              VOID    addpath()       lklibr.c
 *              VOID    bassav()        lkmain.c
 *              VOID    doparse()       lkmain.c
 *              int     fprintf()       c_library
 *              VOID    gblsav()        lkmain.c
 *              VOID    getfid()        lklex.c
 *              int     get()           lklex.c
 *              int     getnb()         lklex.c
 *              VOID    lkexit()        lkmain.c
 *              char *  strsto()        lksym.c
 *              int     strlen()        c_library
 *              int     fndidx()        lkmain.c
 *
 *      side effects:
 *              Various linker flags are updated and the linked
 *              structure lfile is created.
 */

int
parse()
{
        int c;
        int sv_type;
        char fid[NINPUT];

        while ((c = getnb()) != 0) {
                /* sdld specific */
                if ( c == ';')
                        return(0);
                /* end sdld specific */
                if ( c == '-') {
                        while (ctype[c=get()] & LETTER) {
                                switch(c) {

                                case 'C':
                                        if (is_sdld() && !(TARGET_IS_Z80 || TARGET_IS_GB)) {
                                                codesav();
                                                return(0);
                                        }
                                        // else fall through
                                case 'c':
                                        if (startp->f_type != 0)
                                                break;
                                        startp->f_type = F_STD;
                                        doparse();
                                        return(0);

                                case 'f':
                                case 'F':
                                        if (startp->f_type == F_LNK)
                                                return(0);
                                        unget(getnb());
                                        if (*ip == 0)
                                                usage(ER_FATAL);
                                        sv_type = startp->f_type;
                                        startp->f_idp = strsto(ip);
                                        startp->f_idx = fndidx(ip);
                                        startp->f_type = F_LNK;
                                        doparse();
                                        if (sv_type == F_STD) {
                                                cfp = NULL;
                                                sfp = NULL;
                                                startp->f_type = F_STD;
                                                filep = startp;
                                        }
                                        return(0);

                                case 'I':
                                        if (is_sdld() && !(TARGET_IS_Z80 || TARGET_IS_GB)) {
                                                iramsav();
                                                return(0);
                                        }
                                        // else fall through
                                case 'i':
                                        oflag = 1;
                                        break;

                                case 'S':
                                        if (TARGET_IS_8051) {
                                                unget(getnb());
                                                if (ip && *ip)
                                                {
                                                        stacksize = expr(0);
                                                        if (stacksize > 256) stacksize = 256;
                                                        else if (stacksize < 0) stacksize = 0;
                                                }
                                                return(0);
                                        }
                                        // else fall through
                                case 's':
                                        oflag = 2;
                                        break;

                                case 't':
                                case 'T':
                                        oflag = 3;
                                        break;

                                case 'o':
                                case 'O':
                                        objflg = 0;
                                        break;

                                case 'v':
                                case 'V':
                                        objflg = 1;
                                        break;

                                case 'M':
                                        /*JCF: memory usage summary output*/
                                        if (is_sdld()) {
                                                sflag = 1;
                                                break;
                                        }
                                        // else fall through
                                case 'm':
                                        mflag = 1;
                                        break;

#if NOICE
                                case 'j':
                                case 'J':
                                        jflag = 1;
                                        break;
#endif

                                case 'r':
                                case 'R':
                                        if (is_sdld() && !(TARGET_IS_Z80 || TARGET_IS_GB))
                                                rflag = 1;
                                        else
                                                goto err;
                                        break;

                                case 'u':
                                case 'U':
                                        uflag = 1;
                                        break;

                                case 'X':
                                        if (is_sdld() && !(TARGET_IS_Z80 || TARGET_IS_GB)) {
                                                xramsav();
                                                return(0);
                                        }
                                        // else fall through
                                case 'x':
                                        xflag = 0;
                                        break;

                                case 'q':
                                case 'Q':
                                        xflag = 1;
                                        break;

                                case 'd':
                                case 'D':
                                        xflag = 2;
                                        break;

                                case 'E':
                                        if (TARGET_IS_6808) {
                                                oflag = 4;
                                                break;
                                        }
                                        // else fall through
                                case 'e':
                                        return(1);

                                case 'n':
                                case 'N':
                                        pflag = 0;
                                        break;

                                case 'p':
                                case 'P':
                                        pflag = 1;
                                        break;

                                case 'b':
                                case 'B':
                                        bassav();
                                        return(0);

                                case 'g':
                                case 'G':
                                        gblsav();
                                        return(0);

                                case 'k':
                                case 'K':
                                        addpath();
                                        return(0);

                                case 'l':
                                case 'L':
                                        addlib();
                                        return(0);

                                case 'w':
                                case 'W':
                                        wflag = 1;
                                        break;

#if SDCDB
                                case 'Y':
                                        if (TARGET_IS_8051) {
                                                unget(getnb());
                                                packflag=1;
                                                break;
                                        }
                                        // else fall through
                                case 'y':
                                        yflag = 1;
                                        break;
#endif

                                case 'z':
                                case 'Z':
                                        zflag = 1;
                                        break;

                                default:
                                err:
                                        fprintf(stderr,
                                            "Unknown option -%c ignored\n", c);
                                        break;
                                }
                        }
                        /* sdld specific */
                        if ( c == ';')
                                return(0);
                        /* end sdld specific */
                } else
                if (!(ctype[c] & ILL)) {
                        if (linkp == NULL) {
                                linkp = (struct lfile *)
                                        new (sizeof (struct lfile));
                                lfp = linkp;
                                lfp->f_type = F_OUT;
                        } else {
                                lfp->f_flp = (struct lfile *)
                                                new (sizeof (struct lfile));
                                lfp = lfp->f_flp;
                                lfp->f_type = F_REL;
                        }
                        getfid(fid, c);
                        lfp->f_idp = strsto(fid);
                        lfp->f_obj = objflg;
                } else {
                        fprintf(stderr, "Invalid input\n");
                        lkexit(ER_FATAL);
                }
        }
        return(0);
}

/*)Function     VOID    doparse()
 *
 *      The function doparse() evaluates all interactive
 *      command line or file input linker directives and
 *      updates the appropriate variables.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              FILE *  stdin           standard input
 *              FILE *  stdout          standard output
 *              lfile   *cfp            The pointer *cfp points to the
 *                                      current lfile structure
 *              FILE    *sfp            The file handle sfp points to the
 *                                      currently open file
 *              char    ib[NINPUT]      .rel file text line
 *              char    *ip             pointer into the .rel file
 *              lfile   *filep          The pointer *filep points to the
 *                                      beginning of a linked list of
 *                                      lfile structures.
 *              lfile   *startp         asmlnk startup file structure
 *              int     pflag           print linker command file flag
 *
 *      Functions called:
 *              int     fclose()        c_library
 *              int     fprintf()       c_library
 *              VOID    getfid()        lklex.c
 *              int     nxtline()       lklex.c
 *              int     parse()         lkmain.c
 *
 *      side effects:
 *              Various linker flags are updated and the linked
 *              structure lfile may be updated.
 */

VOID
doparse()
{
        cfp = NULL;
        sfp = NULL;
        filep = startp;
        while (1) {
                ip = ib;
                if (nxtline() == 0)
                        break;
                if (pflag && cfp->f_type != F_STD)
                        fprintf(stdout, "ASlink >> %s\n", ip);
                if (*ip == 0 || parse())
                        break;
        }
        if((sfp != NULL) && (sfp != stdin)) {
                fclose(sfp);
        }
        sfp = NULL;
        startp->f_idp = "";
        startp->f_idx = 0;
        startp->f_type = 0;
}

/*)Function     VOID    bassav()
 *
 *      The function bassav() creates a linked structure containing
 *      the base address strings input to the linker.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              base    *basep          The pointer to the first
 *                                      base structure
 *              base    *bsp            Pointer to the current
 *                                      base structure
 *              char    *ip             pointer into the REL file
 *                                      text line in ib[]
 *
 *       functions called:
 *              int     getnb()         lklex.c
 *              VOID *  new()           lksym.c
 *              int     strlen()        c_library
 *              char *  strcpy()        c_library
 *              VOID    unget()         lklex.c
 *
 *      side effects:
 *              The basep structure is created.
 */

VOID
bassav()
{
        if (basep == NULL) {
                basep = (struct base *)
                        new (sizeof (struct base));
                bsp = basep;
        } else {
                bsp->b_base = (struct base *)
                                new (sizeof (struct base));
                bsp = bsp->b_base;
        }
        unget(getnb());
        bsp->b_strp = (char *) new (strlen(ip)+1);
        strcpy(bsp->b_strp, ip);
}


/*)Function     VOID    gblsav()
 *
 *      The function gblsav() creates a linked structure containing
 *      the global variable strings input to the linker.
 *
 *      local variable:
 *              none
 *
 *      global variables:
 *              globl   *globlp         The pointer to the first
 *                                      globl structure
 *              globl   *gsp            Pointer to the current
 *                                      globl structure
 *              char    *ip             pointer into the REL file
 *                                      text line in ib[]
 *              int     lkerr           error flag
 *
 *      functions called:
 *              int     getnb()         lklex.c
 *              VOID *  new()           lksym.c
 *              int     strlen()        c_library
 *              char *  strcpy()        c_library
 *              VOID    unget()         lklex.c
 *
 *      side effects:
 *              The globlp structure is created.
 */

VOID
gblsav()
{
        if (globlp == NULL) {
                globlp = (struct globl *)
                        new (sizeof (struct globl));
                gsp = globlp;
        } else {
                gsp->g_globl = (struct globl *)
                                new (sizeof (struct globl));
                gsp = gsp->g_globl;
        }
        unget(getnb());
        gsp->g_strp = (char *) new (strlen(ip)+1);
        strcpy(gsp->g_strp, ip);
}


/*)Function     VOID    setgbl()
 *
 *      The function setgbl() scans the global variable lines in the
 *      globlp structure, evaluates the arguments, and sets a variable
 *      to this value.
 *
 *      local variables:
 *              int     v               expression value
 *              char    id[]            base id string
 *              sym *   sp              pointer to a symbol structure
 *
 *      global variables:
 *              char    *ip             pointer into the REL file
 *                                      text line in ib[]
 *              globl   *globlp         The pointer to the first
 *                                      globl structure
 *              globl   *gsp            Pointer to the current
 *                                      globl structure
 *              FILE *  stderr          c_library
 *              int     lkerr           error flag
 *
 *       functions called:
 *              a_uint  expr()          lkeval.c
 *              int     fprintf()       c_library
 *              VOID    getid()         lklex.c
 *              int     getnb()         lklex.c
 *              sym *   lkpsym()        lksym.c
 *
 *      side effects:
 *              The value of a variable is set.
 */

VOID
setgbl()
{
        int v;
        struct sym *sp;
        char id[NCPS];

        gsp = globlp;
        while (gsp) {
                ip = gsp->g_strp;
                getid(id, -1);
                if (getnb() == '=') {
                        v = (int) expr(0);
                        sp = lkpsym(id, 0);
                        if (sp == NULL) {
                                fprintf(stderr,
                                "No definition of symbol %s\n", id);
                                lkerr++;
                        } else {
                                if (sp->s_type & S_DEF) {
                                        fprintf(stderr,
                                        "Redefinition of symbol %s\n", id);
                                        lkerr++;
                                        sp->s_axp = NULL;
                                }
                                sp->s_addr = v;
                                sp->s_type |= S_DEF;
                        }
                } else {
                        fprintf(stderr, "No '=' in global expression");
                        lkerr++;
                }
                gsp = gsp->g_globl;
        }
}

/*)Function     FILE *  afile(fn, ft, wf)
 *
 *              char *  fn              file specification string
 *              char *  ft              file type string
 *              int     wf              0 ==>> read
 *                                      1 ==>> write
 *                                      2 ==>> binary write
 *
 *      The function afile() opens a file for reading or writing.
 *              (1)     If the file type specification string ft
 *                      is not NULL then a file specification is
 *                      constructed with the file path\name in fn
 *                      and the extension in ft.
 *              (2)     If the file type specification string ft
 *                      is NULL then the file specification is
 *                      constructed from fn.  If fn does not have
 *                      a file type then the default .rel file
 *                      type is appended to the file specification.
 *
 *      afile() returns a file handle for the opened file or aborts
 *      the assembler on an open error.
 *
 *      local variables:
 *              int     c               character value
 *              FILE *  fp              filehandle for opened file
 *              char *  p1              pointer to filespec string fn
 *              char *  p2              pointer to filespec string fb
 *              char *  p3              pointer to filetype string ft
 *
 *      global variables:
 *              char    afspec[]        constructed file specification string
 *              int     lkerr           error flag
 *
 *      functions called:
 *              int     fndidx()        lkmain.c
 *              FILE *  fopen()         c_library
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              File is opened for read or write.
 */

FILE *
afile(char *fn, char *ft, int wf)
{
        char *p1, *p2;
        int c;
        char * frmt;
        FILE *fp;

        if (strlen(fn) > (FILSPC-7)) {
                fprintf(stderr, "?ASlink-Error-<filspc too long> : \"%s\"\n", fn);
                lkerr++;
                return(NULL);
        }

        /*
         * Skip The Path
         */
        strcpy(afspec, fn);
        c = fndidx(afspec);

        /*
         * Skip to File Extension separator
         */
        p1 = strrchr(&afspec[c], FSEPX);

        /*
         * Copy File Extension
         */
        p2 = ft ? ft : "";
        if (*p2 == 0) {
                if (p1 == NULL) {
                        p2 = LKOBJEXT;
                } else {
                        p2 = strrchr(&fn[c], FSEPX) + 1;
                }
        }
        if (p1 == NULL) {
                p1 = &afspec[strlen(afspec)];
        }
        *p1++ = FSEPX;
        while ((c = *p2++) != 0) {
                if (p1 < &afspec[FILSPC-1])
                        *p1++ = c;
        }
        *p1++ = 0;

        /*
         * Select Read/Write/Binary Write
         */
        switch(wf) {
        default:
        case 0: frmt = "r";     break;
        case 1: frmt = "w";     break;
#ifdef  DECUS
        case 2: frmt = "wn";    break;
#else
        case 2: frmt = "wb";    break;
#endif
        }
        if ((fp = fopen(afspec, frmt)) == NULL && strcmp(ft,"adb") != 0) { /* Do not complain for optional adb files */
                fprintf(stderr, "?ASlink-Error-<cannot %s> : \"%s\"\n", wf?"create":"open", afspec);
                lkerr++;
        }
        return (fp);
}

/*)Function     int     fndidx(str)
 *
 *              char *  str             file specification string
 *
 *      The function fndidx() scans the file specification string
 *      to find the index to the file name.  If the file
 *      specification contains a 'path' then the index will
 *      be non zero.
 *
 *      fndidx() returns the index value.
 *
 *      local variables:
 *              char *  p1              temporary pointer
 *              char *  p2              temporary pointer
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              char *  strrchr()       c_library
 *
 *      side effects:
 *              none
 */

int
fndidx(str)
char *str;
{
        char *p1, *p2;

        /*
         * Skip Path Delimiters
         */
        p1 = str;
        if ((p2 = strrchr(p1,  ':')) != NULL) { p1 = p2 + 1; }
        if ((p2 = strrchr(p1,  '/')) != NULL) { p1 = p2 + 1; }
        if ((p2 = strrchr(p1, '\\')) != NULL) { p1 = p2 + 1; }

        return((int) (p1 - str));
}

/*)Function     int     fndext(str)
 *
 *              char *  str             file specification string
 *
 *      The function fndext() scans the file specification string
 *      to find the file.ext separater.
 *
 *      fndext() returns the index to FSEPX or the end of the string.
 *
 *      local variables:
 *              char *  p1              temporary pointer
 *              char *  p2              temporary pointer
 *
 *      global variables:
 *              none
 *
 *      functions called:
 *              char *  strrchr()       c_library
 *
 *      side effects:
 *              none
 */

int
fndext(str)
char * str;
{
        char *p1, *p2;

        /*
         * Find the file separator
         */
        p1 = str + strlen(str);
        if ((p2 = strrchr(str,  FSEPX)) != NULL) { p1 = p2; }

        return((int) (p1 - str));
}

/* sdld specific */
/*)Function     VOID    iramsav()
 *
 *      The function iramsav() stores the size of the chip's internal RAM.
 *      This is used after linking to check that variable assignment to this
 *      dataspace didn't overflow into adjoining segments.      Variables in the
 *      DSEG, OSEG, and ISEG are assigned to this dataspace.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              char    *ip             pointer into the REL file
 *                                      text line in ib[]
 *              unsigned int            size of chip's internal
 *              iram_size               RAM segment
 *
 *       functions called:
 *              int     getnb()         lklex.c
 *              VOID    unget()         lklex.c
 *              a_uint  expr()          lkeval.c
 *
 *      side effects:
 *              The iram_size may be modified.
 */

VOID
iramsav()
{
  unget(getnb());
  if (ip && *ip)
        iram_size = expr(0);    /* evaluate size expression */
  else
        iram_size = 128;                /* Default is 128 (0x80) bytes */
  if ((iram_size<=0) || (iram_size>256))
        iram_size = 128;                /* Default is 128 (0x80) bytes */
}

/*Similar to iramsav but for xram memory*/
VOID
xramsav()
{
  unget(getnb());
  if (ip && *ip)
        xram_size = expr(0);    /* evaluate size expression */
  else
        xram_size = rflag?0x1000000:0x10000;
}

/*Similar to iramsav but for code memory*/
VOID
codesav()
{
  unget(getnb());
  if (ip && *ip)
        code_size = expr(0);    /* evaluate size expression */
  else
        code_size = rflag?0x1000000:0x10000;
}


/*)Function     VOID    iramcheck()
 *
 *      The function iramcheck() is used at the end of linking to check that
 *      the internal RAM area wasn't overflowed by too many variable
 *      assignments.  Variables in the DSEG, ISEG, and OSEG are assigned to
 *      the chip's internal RAM.
 *
 *      local variables:
 *              none
 *
 *      global variables:
 *              unsigned int            size of chip's internal
 *              iram_size               RAM segment
 *              struct area             linked list of memory
 *              *areap                  areas
 *
 *       functions called:
 *
 *      side effects:
 */

VOID
iramcheck()
{
  register unsigned int last_addr;
  register struct area *ap;

  for (ap = areap; ap; ap=ap->a_ap) {
        if ((ap->a_size != 0) &&
                (!strcmp(ap->a_id, "DSEG") ||
                 !strcmp(ap->a_id, "OSEG") ||
                 !strcmp(ap->a_id, "ISEG")
                )
           )
        {
          last_addr = ap->a_addr + ap->a_size - 1;
          if (last_addr >= iram_size)
                fprintf(stderr,
                  "\nWARNING! Segment %s extends past the end\n"
                  "         of internal RAM.  Check map file.\n",
                  ap->a_id);
        }
  }
}
/* end sdld specific */

char *usetxt[] = {
        "Usage: [-Options] [-Option with arg] file",
        "Usage: [-Options] [-Option with arg] outfile file1 [file2 ...]",
        "Startup:",
        "  -p   Echo commands to stdout (default)",
        "  -n   No echo of commands to stdout",
        "Alternates to Command Line Input:",
        "  -c                   ASlink >> prompt input",
        "  -f   file[.lk]       Command File input",
        "Libraries:",
        "  -k   Library path specification, one per -k",
        "  -l   Library file specification, one per -l",
        "Relocation:",
        "  -b   area base address = expression",
        "  -g   global symbol = expression",
        "Map format:",
        "  -m   Map output generated as (out)file[.map]",
        "  -w   Wide listing format for map file",
        "  -x   Hexadecimal (default)",
        "  -d   Decimal",
        "  -q   Octal",
        "Output:",
        "  -i   Intel Hex as (out)file[.ihx]",
        "  -s   Motorola S Record as (out)file[.s19]",
//      "  -t   Tandy CoCo Disk BASIC binary as (out)file[.bi-]",
#if NOICE
        "  -j   NoICE Debug output as (out)file[.noi]",
#endif
#if SDCDB
        "  -y   SDCDB Debug output as (out)file[.cdb]",
#endif
//      "  -o   Linked file/library object output enable (default)",
//      "  -v   Linked file/library object output disable",
        "List:",
        "  -u   Update listing file(s) with link data as file(s)[.rst]",
        "Case Sensitivity:",
        "  -z   Disable Case Sensitivity for Symbols",
        "End:",
        "  -e   or null line terminates input",
        "",
        0
};

char *usetxt_8051[] = {
        "Usage: [-Options] [-Option with arg] file",
        "Usage: [-Options] [-Option with arg] outfile file1 [file2 ...]",
        "Startup:",
        "  -p   Echo commands to stdout (default)",
        "  -n   No echo of commands to stdout",
        "Alternates to Command Line Input:",
        "  -c                   ASlink >> prompt input",
        "  -f   file[.lk]       Command File input",
        "Libraries:",
        "  -k   Library path specification, one per -k",
        "  -l   Library file specification, one per -l",
        "Relocation:",
        "  -b   area base address = expression",
        "  -g   global symbol = expression",
        "Map format:",
        "  -m   Map output generated as (out)file[.map]",
        "  -w   Wide listing format for map file",
        "  -x   Hexadecimal (default)",
        "  -d   Decimal",
        "  -q   Octal",
        "Output:",
        "  -i   Intel Hex as (out)file[.ihx]",
        "  -s   Motorola S Record as (out)file[.s19]",
#if NOICE
        "  -j   NoICE Debug output as (out)file[.noi]",
#endif
#if SDCDB
        "  -y   SDCDB Debug output as (out)file[.cdb]",
#endif
        "List:",
        "  -u   Update listing file(s) with link data as file(s)[.rst]",
        "Case Sensitivity:",
        "  -z   Disable Case Sensitivity for Symbols",
        "Miscellaneous:\n"
        "  -I   [iram-size] Check for internal RAM overflow",
        "  -X   [xram-size] Check for external RAM overflow",
        "  -C   [code-size] Check for code overflow",
        "  -M   Generate memory usage summary file[.mem]",
        "  -Y   Pack internal ram",
        "  -S   [stack-size] Allocate space for stack",
        "End:",
        "  -e   or null line terminates input",
        "",
        0
};

char *usetxt_6808[] = {
        "Usage: [-Options] [-Option with arg] file",
        "Usage: [-Options] [-Option with arg] outfile file1 [file2 ...]",
        "Startup:",
        "  -p   Echo commands to stdout (default)",
        "  -n   No echo of commands to stdout",
        "Alternates to Command Line Input:",
        "  -c                   ASlink >> prompt input",
        "  -f   file[.lk]       Command File input",
        "Libraries:",
        "  -k   Library path specification, one per -k",
        "  -l   Library file specification, one per -l",
        "Relocation:",
        "  -b   area base address = expression",
        "  -g   global symbol = expression",
        "Map format:",
        "  -m   Map output generated as (out)file[.map]",
        "  -w   Wide listing format for map file",
        "  -x   Hexadecimal (default)",
        "  -d   Decimal",
        "  -q   Octal",
        "Output:",
        "  -i   Intel Hex as (out)file[.ihx]",
        "  -s   Motorola S Record as (out)file[.s19]",
        "  -E   ELF executable as file[.elf]",
#if NOICE
        "  -j   NoICE Debug output as (out)file[.noi]",
#endif
#if SDCDB
        "  -y   SDCDB Debug output as (out)file[.cdb]",
#endif
        "List:",
        "  -u   Update listing file(s) with link data as file(s)[.rst]",
        "Case Sensitivity:",
        "  -z   Disable Case Sensitivity for Symbols",
        "Miscellaneous:\n"
        "  -I   [iram-size] Check for internal RAM overflow",
        "  -X   [xram-size] Check for external RAM overflow",
        "  -C   [code-size] Check for code overflow",
        "  -M   Generate memory usage summary file[.mem]",
        "End:",
        "  -e   or null line terminates input",
        "",
        0
};

char *usetxt_z80_gb[] = {
        "Usage: [-Options] [-Option with arg] file",
        "Usage: [-Options] [-Option with arg] outfile file1 [file2 ...]",
        "Startup:",
        "  -p   Echo commands to stdout (default)",
        "  -n   No echo of commands to stdout",
        "Alternates to Command Line Input:",
        "  -c                   ASlink >> prompt input",
        "  -f   file[.lk]       Command File input",
        "Libraries:",
        "  -k   Library path specification, one per -k",
        "  -l   Library file specification, one per -l",
        "Relocation:",
        "  -b   area base address = expression",
        "  -g   global symbol = expression",
        "Map format:",
        "  -m   Map output generated as (out)file[.map]",
        "  -w   Wide listing format for map file",
        "  -x   Hexadecimal (default)",
        "  -d   Decimal",
        "  -q   Octal",
        "Output:",
        "  -i   Intel Hex as (out)file[.ihx]",
        "  -s   Motorola S Record as (out)file[.s19]",
#if SDCDB
        "  -y   SDCDB Debug output as (out)file[.cdb]",
#endif
        "List:",
        "  -u   Update listing file(s) with link data as file(s)[.rst]",
        "Case Sensitivity:",
        "  -z   Disable Case Sensitivity for Symbols",
        "End:",
        "  -e   or null line terminates input",
        "",
        0
};

/*)Function     VOID    usage(n)
 *
 *              int     n               exit code
 *
 *      The function usage() outputs to the stderr device the
 *      linker name and version and a list of valid linker options.
 *
 *      local variables:
 *              char ** dp              pointer to an array of
 *                                      text string pointers.
 *
 *      global variables:
 *              FILE *  stderr          c_library
 *
 *      functions called:
 *              int     fprintf()       c_library
 *
 *      side effects:
 *              none
 */

VOID
usage(int n)
{
        char    **dp;

        /* sdld specific */
        fprintf(stderr, "\n%s Linker %s\n\n", is_sdld() ? "sdld" : "ASxxxx", VERSION);
        for (dp = TARGET_IS_8051 ? usetxt_8051 : (TARGET_IS_6808 ? usetxt_6808 : ((TARGET_IS_Z80 || TARGET_IS_GB) ? usetxt_z80_gb : usetxt)); *dp; dp++)
                fprintf(stderr, "%s\n", *dp);
        /* end sdld specific */
        lkexit(n);
}

/*)Function     VOID    copyfile()
 *
 *              FILE    *dest           destination file
 *              FILE    *src            source file
 *
 *              function will copy source file to destination file
 *
 *
 *      functions called:
 *              int     fgetc()                 c_library
 *              int     fputc()                 c_library
 *
 *      side effects:
 *              none
 */
VOID
copyfile (FILE *dest, FILE *src)
{
        int ch;

        while ((ch = fgetc(src)) != EOF) {
                fputc(ch,dest);
        }
}
