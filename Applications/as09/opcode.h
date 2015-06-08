/* opcode.h - routine numbers and special opcodes for assembler */

enum
{
/* Pseudo-op routine numbers.
 * Conditionals are first - this is used to test if op is a conditional.
 */
    ELSEOP,
    ELSEIFOP,
    ELSEIFCOP,
    ENDIFOP,
    IFOP,
    IFCOP,

#define MIN_NONCOND	ALIGNOP
    ALIGNOP,
    ASCIZOP,
    BLKWOP,
    BLOCKOP,
    BSSOP,
    COMMOP,
    COMMOP1,
    DATAOP,
    ENDBOP,
    ENTEROP,
    ENTRYOP,
    EQUOP,
    EVENOP,
    EXPORTOP,
    FAILOP,
    FCBOP,
    FCCOP,
    FDBOP,
#if SIZEOF_OFFSET_T > 2
    FQBOP,
#endif
    GETOP,
    GLOBLOP,
    IDENTOP,
    IMPORTOP,
    LCOMMOP,
    LCOMMOP1,
    LISTOP,
    LOCOP,
    MACLISTOP,
    MACROOP,
    MAPOP,
    ORGOP,
    PROCEOFOP,
    RMBOP,
    SECTOP,
    SETOP,
    SETDPOP,
    TEXTOP,
#ifdef I80386
    USE16OP,
    USE32OP,
#endif
    WARNOP,

/* Machine-op routine numbers. */
#ifdef I80386
    BCC,
    BSWAP,
    CALL,
    CALLI,
    DIVMUL,
    ENTER,
    EwGw,
    ExGx,
    F_INHER,
    F_M,
    F_M2,
    F_M2_AX,
    F_M2_M4,
    F_M2_M4_M8,
    F_M4_M8_OPTST,
    F_M4_M8_ST,
    F_M4_M8_STST,
    F_M4_M8_M10_ST,
    F_M10,
    F_OPTST,
    F_ST,
    F_STST,
    F_W_INHER,
    F_W_M,
    F_W_M2,
    F_W_M2_AX,
    GROUP1,
    GROUP2,
    GROUP6,
    GROUP7,
    GROUP8,
    GvEv,
    GvMa,
    GvMp,
    IMUL,
    IN,
    INCDEC,
    INHER,
    INHER16,
    INHER32,
    INHER_A,
    INT,
    JCC,
    JCXZ,
    LEA,
    MOV,
    MOVX,
    NEGNOT,
    OUT,
    PUSHPOP,
    RET,
    SEG,
    SETCC,
    SH_DOUBLE,
    TEST,
    XCHG
#endif /* I80386 */

#ifdef MC6809
    ALL,			/* all address modes allowed, like LDA */
    ALTER,			/* all but immediate, like STA */
    IMMED,			/* immediate only (ANDCC, ORCC) */
    INDEXD,			/* indexed (LEA's) */
    INHER,			/* inherent, like CLC or CLRA */
    LONG,			/* long branches */
    SHORT,			/* short branches */
    SSTAK,			/* S-stack (PSHS, PULS) */
    SWAP,			/* TFR, EXG */
    USTAK			/* U-stack (PSHU,PULU) */
#endif /* MC6809 */
};

/* Special opcodes. */
#ifdef I80386
# define CMP_OPCODE_BASE	0x38
# define CMPSB_OPCODE		0xA6
# define CMPSW_OPCODE		0xA7
# define ESCAPE_OPCODE_BASE	0xD8
# define FST_ENCODED		0x12
# define FSTP_ENCODED		0x13
# define JMP_OPCODE		0xE9
# define JMP_SHORT_OPCODE	0xEB
# define JSR_OPCODE		0xE8
# define MOVSB_OPCODE		0xA4
# define MOVSW_OPCODE		0xA5
# define PAGE1_OPCODE		0x0F
# define POP_OPCODE 		0x8F
# define PUSH_OPCODE 		0xFF
# define WAIT_OPCODE		0x9B
#endif

#ifdef MC6809
# define JMP_OPCODE		0x7E
# define JSR_OPCODE		0xBD
# define PAGE1_OPCODE		0x10
# define PAGE2_OPCODE		0x11
#endif
