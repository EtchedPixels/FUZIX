extern void scm_reset(void);
extern void scm_version(void);
extern void scm_set_irq(void *ptr);
extern void scm_set_nmi(void *ptr);
extern uint8_t scm_input(uint8_t port) __z88dk_fastcall;
extern uint8_t scm_output(uint16_t portchar) __z88dk_fastcall;
extern void scm_poll(void);
extern uint8_t scm_setbaud(uint16_t portbaud) __z88dk_fastcall;
extern uint8_t scm_execute(uint8_t *cmd) __z88dk_fastcall;
extern void scm_setconsole(uint8_t console) __z88dk_fastcall;
extern uint16_t scm_console(void);
extern uint8_t scm_farget(uint8_t *addr) __z88dk_fastcall;
extern uint16_t scm_ramtop(void);
extern void scm_set_ramtop(uint8_t *addr) __z88dk_fastcall;


extern uint16_t scm_monitor_ver;
extern uint16_t scm_config_id;
extern uint16_t scm_hw_info;
extern uint8_t scm_monitor_rev;

#define SCM_SC108	0		/* Not yet defined FIXME */

#define SCM_CUSTOM	0
#define SCM_SIMULATOR	1
#define SCM_DEVKIT01	2
#define SCM_RC2014	3
#define SCM_SC101	4
#define SCM_LiNC80	5
#define SCM_TomsSBC	6
#define SCM_Z280RC	7
#define SCM_SC114	8
#define SCM_SBCRCZ80	9
