
typedef short coord_t;

struct utk_rect
{
    coord_t top;
    coord_t left;
    coord_t bottom;
    coord_t right;
};

extern struct utk_rect clip;
extern struct utk_rect screen;

struct utk_widget {
    struct utk_widget *next;
};

struct utk_menu {
    struct utk_menu *next;
    const char *title;
};

struct utk_window {
    struct utk_window *over;
    struct utk_window *under;
    struct utk_rect rect;
    const char *title;
    struct utk_widget *widget;
    struct utk_menu *menu;
};

extern struct utk_window *win_top, *win_bottom;

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

/* Clip functions */
unsigned clip_covered(struct utk_rect *r);
unsigned clip_outside(struct utk_rect *r);
void clip_intersect(struct utk_rect *r);
void clip_union(struct utk_rect *r);
void clip_save(struct utk_rect *r);
void clip_set(struct utk_rect *r);

void rect_copy(struct utk_rect *to, struct utk_rect *from);
unsigned rect_contains(struct utk_rect *r, coord_t y, coord_t x);

/* Drawing routines */
void utk_init(void);
void utk_win_base(struct utk_window *w);
void utk_render_widget(struct utk_window *w, struct utk_widget *d);
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

/* Events */
struct utk_window *utk_find_window(coord_t y, coord_t x);
struct utk_event *utk_event(void);
