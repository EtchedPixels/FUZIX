/* Error handler */

extern void error(int n);
#define ERROR_SYNTAX		1
#define ERROR_DIVZERO		2
#define ERROR_MEMORY		3
#define ERROR_RET_UNDERFLOW	4
#define ERROR_TYPE		5
#define ERROR_OVERFLOW		6
#define ERROR_RANGE		7
#define ERROR_BADINDEX		8

extern void syntax(void);

/* Parsing */

extern uint8_t *execp;
extern void require(uint8_t c);

/* Scratch buffers */
extern uint8_t *stralloc(unsigned int len);

/* Expressions */

struct value {
    uint8_t type;
#define T_STRING	1
#define T_INT		2
    union {
        int intval;		/* Will become float eventually */
        struct {
            uint8_t *data;
            uint16_t len;
            uint8_t temp;
        } string;
    } d;
};

typedef struct value value;

void expression(struct value *r);
int intexpression(void);
void makenumber(struct value *r);

/* Variables */
void get_variable(struct value *r);
struct variable *find_variable(void);
void set_variable(struct variable *v, struct value *r);
void clear_variables(void);

void execute_line_content(uint8_t *l);



#define LEVEL_0		0
#define LEVEL_1		1
#define LEVEL_2		2
#define LEVEL_3		3
#define LEVEL_4		4
#define LEVEL_5		5
#define LEVEL_6		6
#define LEVEL_7		7
#define LEVEL_8		8
#define LEVEL_9		9
#define LEVEL_10	10
#define LEVEL_11	11
#define LEVEL_12	12
#define LEVEL_13	13
#define LEVEL_14	14
#define LEVEL_15	15
