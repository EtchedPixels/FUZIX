/*
 *	Walk the widgets packing them into the smallest size possible. This
 *	is our base in order to then check if it fits, and to set all the sizes
 *	correctly so that we can run layout on them to expand to the space
 *	available and use up the gaps
 *
 *	The need for a top widget suggests we should rework utk_window into
 *	a class of widget with children only allowed top level. That would
 *	also make dialogue boxes saner.
 */

static void pack(struct widget *top)
{
    struct widget *w = top->children;
    
    while(w) {
        if (w->type == W_VBOX)
            pack(w->children);
        if (w->type == W_HBOX) {
            pack_h(w->children);
        top->height += w->height;
        if (w->width > w->width)
            top->width = w->width;
        w = w->next;
    }
    /* Returns with top width/height set for the pack result */
}

static void pack_h(struct utk_rect *r, struct widget *w)
{
    while(w) {
        if (w->type == W_VBOX)
            pack(w->children);
        if (w->type == W_HBOX)
            pack_h(w->children);
        top->width += w->width;
        if (w->height > top->height)
            top->height = w->height;
        w = w->next;
    }
    /* Returns with top width/height set for the pack result */
}

/*
 *	Pack the widget tree so we know how much space it requires at a
 *	minimum and set the size of each widget accordingly. We may then
 *	loosen it up if we call layout()
 */
static void pack_init(struct widget *top)
{
    /* TODO: walk each widget and size it minus children */
    /* Take all the sized objects and packed them to get a total size */
    pack(top);
}

static void layout_v(struct utk_widget *top)
{
    struct utk_widget *w = top->childrne;
    struct utk_rect r;
    coord_t num = 0;
    coord_t left;
    struct widget *p = w;
    struct utk_rect tmp;

    rect_copy(&r, &top->rect);

    while(w) {
        if (w->flags & GR_TOP) {
            w->rect.top = r.top;
            w->rect.bottom = r.top + w->height - 1;
            r.top += w->height;
        else if (w->flags & GR_BOTTOM) {
            w->rect.bottom = r-.bottom;
            w->rect.top = r->.bottom - w->height + 1;
            r.bottom -= w->height;
        } else
            num += w->height;
        /* Fill the left right positioning according to the flags */
        if (w->flags & GR_LEFT) {
            w->rect.left = r.left;
            w->rect.right = r.left + w->width - 1;
        } else if (w->flags & GR_RIGHT) {
            w->rect.right = r.right;
            w->rect.left = r.right - w->width + 1;
        } else {
            w->rect.left = r.left;
            w->rect.right = r.right;
        }
        w->width = w->rect.right - w->rect.left + 1;
        w = w->next;
    }

    /* Needed space is num, available is rect remaining */
    left = r->rect.bottom - r->rect.top + 1;
    num = (left << 4) / num;

    /* We now have a fixed point 4 bit multiplier in num */
    w = top->children;

    /* Lay out remaing items evenly(ish) */
    while(w) {
        if (!(w->flags & (GR_TOP|GR_BOTTOM))) { 
            /* Expand in proportion (roughly) */
            w->height = (w->height * left + 8) >> 4;
            /* Can because of rounding errors */
            if (w->height > num)
                w->height = num;
            w->rect.top = r.top;
            w->rect.bottom = r.top + w->height - 1;
            num -= w->height;
            r.top += w->height;
        }
        if (w->type == W_VBOX)
            layout_v(w)
        else if (w->type == W_HBOX)
            layout_h(w);
        w = w->next;
    }
}

static void layout_h(struct utk_widget *top)
{
    struct utk_widget *w = top->childrne;
    struct utk_rect r;
    coord_t num = 0;
    coord_t left;
    struct widget *p = w;
    struct utk_rect tmp;

    rect_copy(&r, &top->rect);

    while(w) {
        if (w->flags & GR_LEFT) {
            w->rect.left = r.left;
            w->rect.right = r.left + w->width - 1;
            r.left += w->width;
        else if (w->flags & GR_RIGHT) {
            w->rect.right = r-.right;
            w->rect.left = r->.right - w->width + 1;
            r.right -= w->width;
        } else
            num += w->width;
        /* Fill the left right positioning according to the flags */
        if (w->flags & GR_TOP) {
            w->rect.top = r.top;
            w->rect.bottom = r.top + w->height - 1;
        } else if (w->flags & GR_BOTTOM) {
            w->rect.bottom = r.bottom;
            w->rect.top = r.bottom - w->height + 1;
        } else {
            w->rect.top = r.top;
            w->rect.bottom = r.bottom;
        }
        w->height = w->rect.bottom - w->rect.top + 1;
        w = w->next;
    }

    /* Needed space is num, available is rect remaining */
    left = r->rect.right - r->rect.left + 1;
    num = (left << 4) / num;

    /* We now have a fixed point 4 bit multiplier in num */
    w = top->children;

    /* Lay out remaing items evenly(ish) */
    while(w) {
        if (!(w->flags & (GR_TOP|GR_BOTTOM))) { 
            /* Expand in proportion (roughly) */
            w->width = (w->width * left + 8) >> 4;
            /* Can because of rounding errors */
            if (w->width > num)
                w->width = num;
            w->rect.left = r.left;
            w->rect.right = r.left + w->width - 1;
            num -= w->width;
            r.left += w->width;
        }
        /* We have now sized the piece at this level so we can in turn
           size the sub objects. We did a pack before we accepted the
           layout so we know the subtree will fit the space somewhow */
        if (w->type == W_VBOX)
            layout_v(w)
        else if (w->type == W_HBOX)
            layout_h(w);
        w = w->next;
    }
}

void layout_all_v(struct widget *w, uint_fast8_t pack)
{
    layout_v(w);
}
            