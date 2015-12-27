/* Resource limits only exist on LEVEL_2 systems */

#define NRLIMIT		9

#define RLIMIT_AS	0
#define RLIMIT_CORE	1
#define RLIMIT_CPU	2
#define RLIMIT_DATA	3
#define RLIMIT_FSIZE	4
#define RLIMIT_NOFILE	5
#define RLIMIT_NPROC	6
#define RLIMIT_RSS	7
#define RLIMIT_STACK	8

typedef uint32_t rlimit_t

#define RLIM_INFINITY 0xFFFFFFFFUL

struct rlimit {
    rlim_t rlim_cur;
    rlim_t rlim_max;
};


extern int in_group(uint16_t gid);

extern arg_t _setgroups(void);
extern arg_t _getgroups(void);
extern arg_t _getrlimit(void);
extern arg_t _setrlimit(void);
