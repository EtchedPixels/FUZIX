int bdopen(const char *name, int addflags);
int bdwrite(unsigned int, uint8_t *);
int bdread(unsigned int, uint8_t *);
void bdclose(void);
int fd_open(char *name, int addflags);

extern uint16_t swizzle16(uint32_t v);
extern uint32_t swizzle32(uint32_t v);
extern int swizzling;

extern int swizzling;
extern int swapped;

