#define MODE_EDIT 0
#define MODE_LIST 1
#define MODE_SIZE 2

#define CMDLEN 8

#define DEFAULT_DEV "/dev/hda"

void quit(void);
void list_part(void);
void del_part(void);
void add_part(void);
void help(void);
void write_out(void);
void list_types(void);
void list_part(void);
void set_boot(void);
void set_type(void);

void list_partition(char *dev);

char dev[256]; /* FIXME - should be a #define'd number from header file */
int pFd;
unsigned char partitiontable[512];

typedef struct {
  int cmd;
  char *help;
  void (*func)();
} Funcs;

struct hd_geometry geometry;

