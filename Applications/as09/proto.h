/* extern functions */

/* as.c */
int main(int argc, char *argv[]);
void as_abort(char *message);
void finishup(void);
void initp1p2(void);
void line_zero(void);

/* assemble.c */
void assemble(void);

/* express.c */
void absexpres(void);
void chkabs(void);
void nonimpexpres(void);
void showrelbad(void);
void symabsexpres(void);
void symexpres(void);
void expres(void);
void factor(void);
void scompare(void);

/* genbin.c */
void binheader(void);
void bintrailer(void);
void genbin(void);
void initbin(void);
void putbin(int ch);

/* genlist.c */
char *build_2hex_number(unsigned num, char *where);
char *build_number(unsigned num, unsigned width, char *where);
void warning(char * errorstr);
void error(char * errorstr);
void listline(void);
void writec(char ch);
void writenl(void);
void writeoff(offset_t offset);
void writes(const char *s);
void writesn(const char *s);
void writew(unsigned word);

/* genobj.c */
void accumulate_rmb(offset_t offset);
void flushobj(void);
void genobj(void);
void initobj(void);
void objheader(void);
void objtrailer(void);
void putabs(opcode_pt ch);
void putobj(opcode_pt ch);

/* gensym.c */
void gensym(void);

/* macro.c */
void entermac(struct sym_s *symptr);
void pmacro(void);

/* mops.c */
#ifdef I80386
void mbcc(void);
void mbswap(void);
void mcall(void);
void mcalli(void);
void mdivmul(void);
void menter(void);
void mEwGw(void);
void mExGx(void);
void mf_inher(void);
void mf_m(void);
void mf_m2(void);
void mf_m2_ax(void);
void mf_m2_m4(void);
void mf_m2_m4_m8(void);
void mf_m4_m8_optst(void);
void mf_m4_m8_st(void);
void mf_m4_m8_stst(void);
void mf_m4_m8_m10_st(void);
void mf_m10(void);
void mf_optst(void);
void mf_st(void);
void mf_stst(void);
void mf_w_inher(void);
void mf_w_m(void);
void mf_w_m2(void);
void mf_w_m2_ax(void);
void mgroup1(void);
void mgroup2(void);
void mgroup6(void);
void mgroup7(void);
void mgroup8(void);
void mGvEv(void);
void mGvMa(void);
void mGvMp(void);
void mimul(void);
void min(void);
void mincdec(void);
void minher(void);
void minher16(void);
void minher32(void);
void minhera(void);
void mint(void);
void mjcc(void);
void mjcxz(void);
void mlea(void);
void mmov(void);
void mmovx(void);
void mnegnot(void);
void mout(void);
void mpushpop(void);
void mret(void);
void mseg(void);
void msetcc(void);
void mshdouble(void);
void mtest(void);
void mxchg(void);
#endif				/* I80386 */

#ifdef MC6809
void mall(void);
void malter(void);
void mimmed(void);
void mindex(void);
void minher(void);
void mlong(void);
void msstak(void);
void mswap(void);
void mustak(void);
#endif				/* MC6809 */

void getcomma(void);
void mshort(void);

/* pops.c */
bool_pt checksegrel(struct sym_s *symptr);
void checkdatabounds(void);
void datatoobig(void);
void fatalerror(char * errorstr);
void labelerror(char * errorstr);
void palign(void);
void pasciz(void);
void pblkw(void);
void pblock(void);
void pbss(void);
void pcomm(void);
void pcomm1(void);
void pdata(void);
void pelse(void);
void pelseif(void);
void pelsifc(void);
void pendb(void);
void pendif(void);
void penter(void);
void pentry(void);
void pequ(void);
void peven(void);
void pexport(void);
void pfail(void);
void pfcb(void);
void pfcc(void);
void pfdb(void);
#if SIZEOF_OFFSET_T > 2
void pfqb(void);
#endif
void pglobl(void);
void pident(void);
void pif(void);
void pifc(void);
void pimport(void);
void plcomm(void);
void plcomm1(void);
void plist(void);
void pnolist(void);
void ploc(void);
void pmaclist(void);
void pmap(void);
void porg(void);
void prmb(void);
void psect(void);
void pset(void);
void psetdp(void);
void ptext(void);
void puse16(void);
void puse32(void);
void pwarn(void);
void showlabel(void);

/* readsrc.c */
void initsource(void);
fd_t open_input(char *name);
void pget(void);
void pproceof(void);
void readline(void);
void skipline(void);

/* scan.c */
void context_hexconst(void);
void getsym(void);
void getsym_nolookup(void);
void initscan(void);

/* table.c */
void inst_keywords(void);
struct sym_s *lookup(void);
void statistics(void);

/* type.c */
#if defined(__m6809__) && defined(MC6809)
#define NATIVE_ENDIAN
extern inline u2_pt c2u2(char *buf) { return *((u2_pt *)buf); }
extern inline u4_t c4u4(char *buf) { return *((u4_t *)buf); }
extern inline void u2c2(char *buf, u2_t offset) { *((unsigned short *)buf) = offset;}
extern inline void u4c4(char *buf, u4_t offset) { *((unsigned long *)buf) = offset;}
#else
u2_pt c2u2(char *buf);
u4_t c4u4(char *buf);
void u2c2(char *buf, u2_t offset);
void u4c4(char *buf, u32_T offset);
#endif
u2_pt cnu2(char *buf, unsigned count);
u4_t cnu4(char *buf, unsigned count);
void u2cn(char *buf, u16_pt offset, unsigned count);
void u4cn(char *buf, u32_T offset, unsigned count);
bool_pt typeconv_init(bool_pt big_endian, bool_pt long_big_endian);

/* alloc.c */
void * asalloc(unsigned int size);
void * asrealloc(void * oldptr, unsigned int size);
void * temp_buf(void);
void init_heap(void);

