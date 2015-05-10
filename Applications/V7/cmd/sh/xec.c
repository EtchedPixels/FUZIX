/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier. All rights reserved. */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

static INT	parent;

extern SYSTAB		commands;



/* ========	command execution	========*/


execute(argt, execflg, pf1, pf2)
	TREPTR		argt;
	INT		*pf1, *pf2;
{
	/* `stakbot' is preserved by this routine */
	REG TREPTR	t;
	STKPTR		sav=savstak();

	sigchk();

	IF (t=argt) && execbrk==0
	) {	REG INT		treeflgs;
		INT		oldexit, type;
		REG STRING	*com;

		treeflgs = t->tretyp; type = treeflgs&COMMSK;
		oldexit=exitval; exitval=0;

		switch(type) {

		case TCOM:
			{
			STRING		a1;
			INT		argn, internal;
			ARGPTR		schain=gchain;
			IOPTR		io=t->treio;
			gchain=0;
			argn = getarg(t);
			com=scan(argn);
			a1=com[1]; gchain=schain;

			IF (internal=syslook(com[0],commands)) || argn==0
			) {	setlist(((COMPTR)t)->comset, 0);
			;}

			IF argn && (flags&noexec)==0
			) {	/* print command if execpr */
				IF flags&execpr
				) {	argn=0;	prs(execpmsg);
					WHILE com[argn]!=ENDARGS
					DO prs(com[argn++]); blank() OD
					newline();
				;}

				switch(internal) {

				case SYSDOT:
					IF a1
					) {	REG INT		f;
	
						IF (f=pathopen(getpath(a1), a1)) < 0
						) { failed(a1,notfound);
						} else { execexp(0,f);
						;}
					;}
					break;
	
				case SYSTIMES:
					{
					L_INT	t[4]; times(t);
					prt(t[2]); blank(); prt(t[3]); newline();
					}
					break;
	
				case SYSEXIT:
					exitsh(a1?stoi(a1):oldexit);
	
				case SYSNULL:
					io=0;
					break;
	
				case SYSCONT:
					execbrk = -loopcnt; break;
	
				case SYSBREAK:
					IF (execbrk=loopcnt) && a1
					) { breakcnt=stoi(a1);
					;}
					break;
	
				case SYSTRAP:
					IF a1
					) {	BOOL	clear;
						IF (clear=digit(*a1))==0
						) {	++com;
						;}
						WHILE *++com
						DO INT	i;
						   IF (i=stoi(*com))>=MAXTRAP || i<MINTRAP
						   ) {	failed(*com,badtrap);
						   } else if ( clear
						   ) {	clrsig(i);
						   } else {	replace(&trapcom[i],a1);
							IF *a1
							) {	getsig(i);
							} else {	ignsig(i);
							;}
						   ;}
						OD
					} else {	/* print out current traps */
						INT		i;
	
						FOR i=0; i<MAXTRAP; i++
						DO IF trapcom[i]
						   ) {	prn(i); prs(colon); prs(trapcom[i]); newline();
						   ;}
						OD
					;}
					break;
	
				case SYSEXEC:
					com++;
					initio(io); ioset=0; io=0;
					IF a1==0 ) { break ;}
	
				case SYSLOGIN:
					flags |= forked;
					oldsigs(); execa(com); done();
	
				case SYSCD:
					IF flags&rshflg
					) {	failed(com[0],restricted);
					} else if ( (a1==0 && (a1=homenod.namval)==0) || chdir(a1)<0
					) {	failed(a1,baddir);
					;}
					break;
	
				case SYSSHFT:
					IF dolc<1
					) {	error(badshift);
					} else {	dolv++; dolc--;
					;}
					assnum(&dolladr, dolc);
					break;
	
				case SYSWAIT:
					await(-1);
					break;
	
				case SYSREAD:
					exitval=readvar(&com[1]);
					break;

/*
				case SYSTST:
					exitval=testcmd(com);
					break;
*/

				case SYSSET:
					IF a1
					) {	INT	argc;
						argc = options(argn,com);
						IF argc>1
						) {	setargs(com+argn-argc);
						;}
					} else if ( ((COMPTR)t)->comset==0
					) {	/*scan name chain and print*/
						namscan(printnam);
					;}
					break;
	
				case SYSRDONLY:
					exitval=N_RDONLY;
				case SYSXPORT:
					IF exitval==0 ) { exitval=N_EXPORT; ;}
	
					IF a1
					) {	WHILE *++com
						DO attrib(lookup(*com), exitval) OD
					} else {	namscan(printflg);
					;}
					exitval=0;
					break;
	
				case SYSEVAL:
					IF a1
					) {	execexp(a1,&com[2]);
					;}
					break;

                                case SYSUMASK:
                                        if (a1) {
                                                int c, i;
                                                i = 0;
                                                while ((c = *a1++) >= '0' &&
                                                        c <= '7')
                                                        i = (i << 3) + c - '0';
                                                umask(i);
                                        } else {
                                                int i, j;
                                                umask(i = umask(0));
                                                prc('0');
                                                for (j = 6; j >= 0; j -= 3)
                                                        prc(((i>>j)&07) + '0');
                                                newline();
                                        }
                                        break;
	
				default:
					internal=builtin(argn,com);

				}

				IF internal
				) {	IF io ) { error(illegal) ;}
					chktrap();
					break;
				;}
			} else if ( t->treio==0
			) {	break;
			;}
			}
	
		case TFORK:
			IF execflg && (treeflgs&(FAMP|FPOU))==0
			) {	parent=0;
			} else {	WHILE (parent=fork()) == -1
				DO sigchk(); alarm(10); pause() OD
			;}

			IF parent
			) {	/* This is the parent branch of fork;    */
				/* it may or may not wait for the child. */
				IF treeflgs&FPRS && flags&ttyflg
				) {	prn(parent); newline();
				;}
				IF treeflgs&FPCL ) { closepipe(pf1) ;}
				IF (treeflgs&(FAMP|FPOU))==0
				) {	await(parent);
				} else if ( (treeflgs&FAMP)==0
				) {	post(parent);
				} else {	assnum(&pcsadr, parent);
				;}

				chktrap();
				break;


			} else {	/* this is the forked branch (child) of execute */
				flags |= forked; iotemp=0;
				postclr();
				settmp();

				/* Turn off INTR and QUIT if `FINT'  */
				/* Reset ramaining signals to parent */
				/* except for those `lost' by trap   */
				oldsigs();
				IF treeflgs&FINT
				) {	signal(INTR,1); signal(QUIT,1);
				;}

				/* pipe in or out */
				IF treeflgs&FPIN
				) {	sh_rename(pf1[INPIPE],0);
					close(pf1[OTPIPE]);
				;}
				IF treeflgs&FPOU
				) {	sh_rename(pf2[OTPIPE],1);
					close(pf2[INPIPE]);
				;}

				/* default std input for & */
				IF treeflgs&FINT && ioset==0
				) {	sh_rename(chkopen(devnull),0);
				;}

				/* io redirection */
				initio(t->treio);
				IF type!=TCOM
				) {	execute(((FORKPTR)t)->forktre,1);
				} else if ( com[0]!=ENDARGS
				) {	setlist(((COMPTR)t)->comset,N_EXPORT);
					execa(com);
				;}
				done();
			;}

		case TPAR:
			sh_rename(dup(2),output);
			execute(((PARPTR)t)->partre,execflg);
			done();

		case TFIL:
			{
			   INT pv[2]; chkpipe(pv);
			   IF execute(((LSTPTR)t)->lstlef, 0, pf1, pv)==0
			   ) {	execute(((LSTPTR)t)->lstrit, execflg, pv, pf2);
			   } else {	closepipe(pv);
			   ;}
			}
			break;

		case TLST:
			execute(((LSTPTR)t)->lstlef,0);
			execute(((LSTPTR)t)->lstrit,execflg);
			break;

		case TAND:
			IF execute(((LSTPTR)t)->lstlef,0)==0
			) {	execute(((LSTPTR)t)->lstrit,execflg);
			;}
			break;

		case TORF:
			IF execute(((LSTPTR)t)->lstlef,0)!=0
			) {	execute(((LSTPTR)t)->lstrit,execflg);
			;}
			break;

		case TFOR:
			{
			   NAMPTR	n = lookup(((FORPTR)t)->fornam);
			   STRING	*args;
			   DOLPTR	argsav=0;

			   IF ((FORPTR)t)->forlst==0
			   ) {    args=dolv+1;
				   argsav=useargs();
			   } else {	   ARGPTR	schain=gchain;
				   gchain=0;
				   trim((args=scan(getarg(((FORPTR)t)->forlst)))[0]);
				   gchain=schain;
			   ;}
			   loopcnt++;
			   WHILE *args!=ENDARGS && execbrk==0
			   DO	assign(n,*args++);
				execute(((FORPTR)t)->fortre,0);
				IF execbrk<0 ) { execbrk=0 ;}
			   OD
			   IF breakcnt ) { breakcnt-- ;}
			   execbrk=breakcnt; loopcnt--;
			   argfor=freeargs(argsav);
			}
			break;

		case TWH:
		case TUN:
			{
			   INT		i=0;

			   loopcnt++;
			   WHILE execbrk==0 && (execute(((WHPTR)t)->whtre,0)==0)==(type==TWH)
			   DO i=execute(((WHPTR)t)->dotre,0);
			      IF execbrk<0 ) { execbrk=0 ;}
			   OD
			   IF breakcnt ) { breakcnt-- ;}
			   execbrk=breakcnt; loopcnt--; exitval=i;
			}
			break;

		case TIF:
			IF execute(((IFPTR)t)->iftre,0)==0
			) {	execute(((IFPTR)t)->thtre,execflg);
			} else {	execute(((IFPTR)t)->eltre,execflg);
			;}
			break;

		case TSW:
			{
			   REG STRING	r = mactrim(((SWPTR)t)->swarg);
			   t=(TREPTR)((SWPTR)t)->swlst;
			   WHILE t
			   DO	ARGPTR		rex=((REGPTR)t)->regptr;
				WHILE rex
				DO	REG STRING	s;
					IF gmatch(r,s=macro(rex->argval)) || (trim(s), eq(r,s))
					) {	execute(((REGPTR)t)->regcom,0);
						t=0; break;
					} else {	rex=((ARGPTR)rex)->argnxt;
					;}
				OD
				IF t ) { t=(TREPTR)((REGPTR)t)->regnxt ;}
			   OD
			}
			break;
		}
		exitset();
	;}

	sigchk();
	tdystak(sav);
	return(exitval);
}


execexp(s,f)
	STRING		s;
	UFD		f;
{
	FILEBLK		fb;
	push(&fb);
	IF s
	) {	estabf(s); fb.feval=(STRING *)f;
	} else if ( f>=0
	) {	initf(f);
	;}
	execute(cmd(NL, NLFLG|MTFLG),0);
	pop();
}
