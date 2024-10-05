

/* Must match the order in tape_op in the asm */
#define TAPE_WRITE	0
#define TAPE_READ	1
#define TAPE_CLOSEW	2
#define TAPE_REWIND	3
#define TAPE_FIND	4
#define TAPE_SELECT	5
#define TAPE_ERASE	6

extern int tape_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int tape_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int tape_ioctl(uint8_t minor, uarg_t op, char *ptr);
extern int tape_open(uint8_t minor, uint16_t flag);
extern int tape_close(uint8_t minor);
extern void tape_init(void);

extern int tape_op(uint8_t id, uint8_t op);

