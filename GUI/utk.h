/*
 *	Drawing ansd widget structure
 */
 
typedef short coord_t;

struct utk_rect
{
    coord_t top;
    coord_t left;
    coord_t bottom;
    coord_t right;
};

extern struct utk_rect clip;
extern struct utk_rect damage;
extern struct utk_rect screen;

struct utk_menu {
    struct utk_menu *next;
    const char *title;
    const char *items;
    coord_t width;
    coord_t dropheight;
    coord_t dropwidth;
};

struct utk_widget {
    struct utk_widget *next;
    struct utk_rect rect;
    unsigned type;
#define UTK_WINDOW	1
#define UTK_VBOX	2
#define UTK_HBOX	3
    unsigned short height, width;
};

struct utk_container {
    struct utk_widget w;
    struct utk_widget *children;
};

struct utk_window {
    struct utk_container c;
    struct utk_window *over;
    struct utk_window *under;
    const char *title;
    struct utk_menu *menu;
};

extern struct utk_window *win_top, *win_bottom;

/* Event handling */
struct utk_event {
    unsigned type;
    unsigned code;
    coord_t y;
    coord_t x;
    struct utk_window *window;
    struct utk_widget *widget;
};

#define EV_NONE		0
#define EV_QUIT		1
#define EV_KEY		2
#define EV_BUTTON	3
#define EV_CLOSE	4
#define EV_SELECT	5
#define EV_MOVE		6
#define EV_RESIZE	7
#define EV_MENU		8

/*
 *	Functionality description
 */

struct utk_cap {
    coord_t	screen_height;	/* Height of display */
    coord_t	screen_width;	/* Width of display */
    coord_t	xsnap;		/* Objects are aligned on this boundary */
    coord_t	ysnap;		/* Objects are aligned on this boundary */

    coord_t	fonth;		/* Fixed font sizes in coordinates */
    coord_t	fontw;		/* May be 1,1 for character video */

    unsigned	capabilities;
#define CAP_BITMAP	1	/* Supports pixel level graphic ops */
#define CAP_SYSICON	2	/* Supports system icons */
#define CAP_COLOUR	4	/* Supports colour operation */
#define CAP_MULTI	8	/* Multiple apps sharing one desktop */
#define CAP_FONT	16	/* Supports font setting */
#define CAP_VFONT	32	/* Supports variable width fonts */
#define CAP_POINTER	64	/* We have a pointer */
};

extern struct utk_cap utk_cap;

/*
 *	Fonts (sketch for)
 */

struct utk_font {
    const char	*name;
    unsigned	flags;
#define FONT_MONO	1
    coord_t	width;		/* Monospaced only */
    coord_t	height;
};

/* Clip functions */
unsigned clip_covered(struct utk_rect *r);
unsigned clip_outside(struct utk_rect *r);
void clip_intersect(struct utk_rect *r);
void clip_union(struct utk_rect *r);
void clip_save(struct utk_rect *r);
void clip_set(struct utk_rect *r);
void damage_set(struct utk_rect *r);

void rect_copy(struct utk_rect *to, struct utk_rect *from);
unsigned rect_contains(struct utk_rect *r, coord_t y, coord_t x);

/* Text functions */
struct utk_font *utk_font_set(const char *name);
struct utk_font *utk_font_load(const char *name);
void utk_font_ref(struct utk_font *font);
void utk_font_uref(struct utk_font *font);
coord_t utk_text_width(const char *p);
coord_t utk_text_height(const char *p);
coord_t utk_putsn(coord_t y, coord_t x, const char *t, const char *e);
#define utk_puts(y,x,t)	utk_putsn(y, x, t, NULL)

/* Drawing routines */
void utk_init(void);
void utk_exit(void);
void utk_win_base(struct utk_window *w);
void utk_menu(void);
void utk_render_widget(struct utk_widget *w);
void utk_fill_background(void);
void utk_refresh(void);

/* UI routines */
void ui_redraw(void);
void ui_redraw_all(void);
void ui_win_delete(struct utk_window *w);
void ui_win_create(struct utk_window *w);
void ui_win_adjust(struct utk_window *w, struct utk_rect *r);
void ui_win_back(struct utk_window *w);
void ui_win_top(struct utk_window *w);
void ui_init(void);
void ui_exit(void);

/* Events */
struct utk_window *utk_find_window(coord_t y, coord_t x);
struct utk_event *utk_event(void);
