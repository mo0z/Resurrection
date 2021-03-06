/*
 * pager.c - Resurrection X11 window manager pager routines.
 * Copyright (C) 2006 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <Resurrection/Resurrection.h>

void Rwm_pager_label_buttonpress_handler(void *arg,
                                         XEvent *event);
void Rwm_pager_label_buttonrelease_handler(void *arg,
                                           XEvent *event);
void Rwm_pager_click_func(void *arg,
                          XEvent *event,
                          void *bindarg);

struct R_binding Rwm_pager_bindings[] = {
    {
        R_MAKE_BINDING(0,
                       R_BINDING_BUTTON1_FLAG),
        Rwm_pager_click_func,
        &Rwm_desktop_args[0],
        NULL
    },
    {
        R_MAKE_BINDING(0,
                       R_BINDING_BUTTON1_FLAG),
        Rwm_pager_click_func,
        &Rwm_desktop_args[1],
        NULL
    },
    {
        R_MAKE_BINDING(0,
                       R_BINDING_BUTTON1_FLAG),
        Rwm_pager_click_func,
        &Rwm_desktop_args[2],
        NULL
    },
    {
        R_MAKE_BINDING(0,
                       R_BINDING_BUTTON1_FLAG),
        Rwm_pager_click_func,
        &Rwm_desktop_args[3],
        NULL
    }
};

int
Rwm_init_pager(struct R_app *app)
{
    int i;
    
    if (!Rwm_create_pager_windows(app)) {
        
        return FALSE;
    }
    i = 0;
#if (USE_IMLIB2)
    while (i < RWM_DESKTOPS) {
        if (!Rwm_create_pager_thumb(app,
                                    i)) {
            
            return FALSE;
        }
        i++;
    }
#endif
    if (!Rwm_init_pager_bindings(app)) {

        return FALSE;
    }
    Rwm_add_pager(app);
    Rwm_map_pager(app);

    return TRUE;
}

int
Rwm_create_pager_windows(struct R_app *app)
{
    struct R_window *pager;
    struct R_window *label;
    struct R_image *image;
    struct R_window *root;
    struct R_wm     *wm;
    const char      *filename;
#if (USE_NEW_MENU)
    struct R_window *child;
    struct R_window *tmp;
    int flags;
    int x;
    int y;
    int w;
    int h;
    int i;
#endif
    
    root = app->window;
    wm = app->client;
    pager = R_create_window(app,
                            root,
                            R_WINDOW_OVERRIDE_REDIRECT_FLAG
                            | R_WINDOW_SAVE_UNDER_FLAG);
    if (pager == NULL) {
        
        return FALSE;
    }
#if (USE_NEW_MENU)
    Rwm_move_resize_window(pager,
                           RWM_MENU_ITEM_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                           0,
                           RWM_MENU_ITEM_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                           RWM_MENU_ITEM_HEIGHT);
#else
    Rwm_move_resize_window(pager,
                           RWM_MENU_ITEM_WIDTH,
                           0,
                           RWM_PAGER_THUMB_WIDTH,
                           RWM_MENU_ITEM_HEIGHT);
#endif
    label = R_create_window(app,
                            pager,
                            R_WINDOW_OVERRIDE_REDIRECT_FLAG);
    if (label == NULL) {

        return FALSE;
    }
#if (USE_IMLIB2)
    label->font = _menufont;
    image = R_load_image_imlib2(app,
                                RESURRECTION_IMAGE_SEARCH_PATH "widget/winh.png",
                                NULL);
    if (image == NULL) {

        return FALSE;
    }
    image->w = R_TILE_FIT_IMAGE;
    image->h = RWM_MENU_ITEM_HEIGHT;
    label->images[R_WINDOW_NORMAL_STATE] = image;
    label->image = image;
    label->str = "desk";
    image = R_load_image_imlib2(app,
                                RESURRECTION_IMAGE_SEARCH_PATH "widget/winselh.png",
                                NULL);
    if (image == NULL) {

        return FALSE;
    }
    image->w = R_TILE_FIT_IMAGE;
    image->h = RWM_MENU_ITEM_HEIGHT;
    label->images[R_WINDOW_ACTIVE_STATE] = image;

#endif
#if (USE_NEW_MENU)
    Rwm_move_resize_window(label,
                           0,
                           0,
                           RWM_MENU_ITEM_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                           RWM_MENU_ITEM_HEIGHT);
#else
    Rwm_move_resize_window(label,
                           0,
                           0,
                           RWM_MENU_ITEM_WIDTH,
                           RWM_MENU_ITEM_HEIGHT);
#endif
    Rwm_draw_text(label);
    if (wm->optflags & RWM_CLICK_MENU_FLAG) {
        R_set_window_event_handler(label,
                                   ButtonPress,
                                   Rwm_pager_label_buttonpress_handler);
        R_set_window_event_handler(label,
                                   ButtonRelease,
                                   Rwm_pager_label_buttonrelease_handler);
        R_add_window_events(label,
                            ButtonPressMask
                            | ButtonReleaseMask);
//        R_add_window(label);
    }
    pager->typeflags = R_WINDOW_PAGER_FLAG;
    pager->chain = label;
    pager->last = label;
    wm->pager = pager;

#if (USE_NEW_MENU)
    flags = R_WINDOW_STATIC_IMAGES_FLAG;
    /* left */
    w = RWM_MENU_RULER_WIDTH;
    h = RWM_DESKTOPS * (RWM_PAGER_THUMB_HEIGHT + RWM_MENU_RULER_HEIGHT)
        + RWM_MENU_RULER_HEIGHT;

    child = R_create_window(app,
                            pager,
                            0);
    Rwm_move_resize_window(child,
                           0,
                           RWM_MENU_ITEM_HEIGHT,
                           w,
                           h);
    image = R_load_image_imlib2(app,
                                RESURRECTION_IMAGE_SEARCH_PATH "widget/winselv.png",
                                NULL);
    image->w = RWM_MENU_RULER_WIDTH;
    image->h = R_TILE_FIT_IMAGE;
    child->images[R_WINDOW_NORMAL_STATE] = image;
    RWM_SET_WINDOW_IMAGE(child, R_WINDOW_NORMAL_STATE);
    R_set_background_imlib2(image,
                            child,
                            w,
                            h,
                            0);
    pager->child = child;
    tmp = child;
    /* right */
    child = R_create_window(app,
                            pager,
                            0);
    Rwm_move_resize_window(child,
                           RWM_MENU_RULER_WIDTH + RWM_MENU_ITEM_WIDTH,
                           RWM_MENU_ITEM_HEIGHT,
                           w,
                           h);
    image = R_load_image_imlib2(app,
                                RESURRECTION_IMAGE_SEARCH_PATH "widget/winselv.png",
                                NULL);
    image->w = RWM_MENU_RULER_WIDTH;
    image->h = R_TILE_FIT_IMAGE;
    child->images[R_WINDOW_NORMAL_STATE] = image;
    RWM_SET_WINDOW_IMAGE(child, R_WINDOW_NORMAL_STATE);
    R_set_background_imlib2(image,
                            child,
                            w,
                            h,
                            0);
    tmp->chain = child;
    tmp = child;
    /* horizontal rulers. */
    x = 0;
    y = RWM_MENU_ITEM_HEIGHT;
    w = RWM_MENU_ITEM_WIDTH + 2 * RWM_MENU_RULER_WIDTH;
    h = RWM_MENU_RULER_HEIGHT;
    i = 0;
    while (i < RWM_DESKTOPS + 1) {
        child = R_create_window(app,
                                pager,
                                0);
        Rwm_move_resize_window(child,
                               x,
                               y,
                               w,
                               h);
        image = R_load_image_imlib2(app,
                                    RESURRECTION_IMAGE_SEARCH_PATH "widget/winselh.png",
                                    NULL);
        image->w = R_TILE_FIT_IMAGE;
        image->h = RWM_MENU_RULER_HEIGHT;
        child->images[R_WINDOW_NORMAL_STATE] = image;
        RWM_SET_WINDOW_IMAGE(child, R_WINDOW_NORMAL_STATE);
        R_set_background_imlib2(image,
                                child,
                                w,
                                h,
                                0);
        y += RWM_MENU_RULER_HEIGHT + RWM_PAGER_THUMB_HEIGHT;
        tmp->chain = child;
        tmp = child;
        i++;
    }
#endif

    return TRUE;
}

#if (USE_IMLIB2)
int
Rwm_create_pager_thumb(struct R_app *app,
                       int id)
{
    struct R_wm *wm;
    struct R_window *desktop;
    struct R_window *pager;
    struct R_window *thumb;
    struct R_image *image;

    wm = app->client;
    desktop = wm->desktops[id];
    image = desktop->image;
    pager = wm->pager;
    thumb = R_alloc_window();
    thumb->app = app;
    R_init_window(app,
                  thumb,
                  pager,
                  R_WINDOW_OVERRIDE_REDIRECT_FLAG);
#if (USE_NEW_MENU)
    Rwm_move_resize_window(thumb,
                           RWM_MENU_RULER_WIDTH,
                           RWM_MENU_ITEM_HEIGHT
                           + RWM_MENU_RULER_HEIGHT
                           + id * (RWM_PAGER_THUMB_HEIGHT + RWM_MENU_RULER_HEIGHT),
                           RWM_PAGER_THUMB_WIDTH,
                           RWM_PAGER_THUMB_HEIGHT);
#else
    Rwm_move_resize_window(thumb,
                           0,
                           RWM_MENU_ITEM_HEIGHT
                           + id * RWM_PAGER_THUMB_HEIGHT,
                           RWM_PAGER_THUMB_WIDTH,
                           RWM_PAGER_THUMB_HEIGHT);
#endif
    R_set_thumb_imlib2(image,
                       thumb,
                       RWM_PAGER_THUMB_WIDTH,
                       RWM_PAGER_THUMB_HEIGHT,
                       0);
    thumb->app = app;
    thumb->desktop = desktop;
    thumb->image = image;
    pager->last->chain = thumb;
    pager->last = thumb;
    
    return TRUE;
}
#endif

int
Rwm_init_pager_bindings(struct R_app *app)
{
    struct R_wm *wm;
    struct R_window *pager;
    struct R_window *thumb;
    struct R_binding *binding;
    struct R_binding *pagerbinding;

    wm = app->client;
    pager = wm->pager;
    thumb = pager->chain;
    thumb = thumb->chain;
    pagerbinding = &Rwm_pager_bindings[0];
    while (thumb) {
        binding = R_create_binding(thumb,
                                   pagerbinding->mask,
                                   pagerbinding->func,
                                   pagerbinding->arg);
        if (binding == NULL) {
            
            return FALSE;
        }
        if (!R_add_binding(binding,
                           thumb)) {
            
            return FALSE;
        }
        thumb = thumb->chain;
        pagerbinding++;
    }
    
    return TRUE;
}

void
Rwm_init_pager_events(struct R_app *app)
{
    struct R_wm     *wm;
    struct R_window *pager;
    struct R_window *thumb;
    long             mask;

    wm = app->client;
    pager = wm->pager;
    R_set_window_event_handler(pager,
                               EnterNotify,
                               Rwm_pager_enternotify_handler);
    R_set_window_event_handler(pager,
                               LeaveNotify,
                               Rwm_pager_leavenotify_handler);
    thumb = pager->chain;
    while (thumb) {
        mask = R_add_window_events(thumb,
                                   ButtonPressMask);
        thumb = thumb->chain;
    }
    mask = R_add_window_events(pager,
                               EnterWindowMask
                               | LeaveWindowMask);

    return;
}

void
Rwm_add_pager(struct R_app *app)
{
    struct R_wm *wm;
    struct R_window *window;

    wm = app->client;
    window = wm->pager;
    R_add_window(window);
    window = window->chain;
    while (window) {
        R_add_window(window);
        window = window->chain;
    }
    
    return;
}

void
Rwm_map_pager(struct R_app *app)
{
    struct R_wm *wm;
    struct R_window *pager;
    
    wm = app->client;
    pager = wm->pager;
#if 0
    Rwm_reparent_window(pager,
                        wm->desktop,
                        RWM_MENU_ITEM_WIDTH,
                        0);
#endif
    R_map_subwindows(pager);
    Rwm_resize_window(pager,
                      RWM_MENU_ITEM_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                      RWM_MENU_ITEM_HEIGHT);
    R_map_window(pager);
    
    return;
}

void
Rwm_show_pager(struct R_window *pager)
{
    R_raise_window(pager);
#if (USE_NEW_MENU)
    Rwm_resize_window(pager,
                      RWM_PAGER_THUMB_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                      RWM_MENU_ITEM_HEIGHT
                      + RWM_MENU_RULER_HEIGHT
                      + RWM_DESKTOPS * (RWM_PAGER_THUMB_HEIGHT + RWM_MENU_RULER_HEIGHT));
#else
    Rwm_resize_window(pager,
                      RWM_PAGER_THUMB_WIDTH,
                      RWM_MENU_ITEM_HEIGHT
                      + RWM_DESKTOPS * RWM_PAGER_THUMB_HEIGHT);
#endif

    return;
}

void
Rwm_hide_pager(struct R_window *pager)
{
        R_raise_window(pager);
#if (USE_NEW_MENU)
        Rwm_resize_window(pager,
                      RWM_PAGER_THUMB_WIDTH + 2 * RWM_MENU_RULER_WIDTH,
                      RWM_MENU_ITEM_HEIGHT);
#else
        Rwm_resize_window(pager,
                      RWM_PAGER_THUMB_WIDTH,
                      RWM_MENU_ITEM_HEIGHT);
#endif
    
    return;
}

void
Rwm_pager_enternotify_handler(void *arg,
                              XEvent *event)
{
    struct R_window *pager;
    struct R_window *label;
    struct R_wm     *wm;

    pager = arg;
    label = pager->chain;
    wm = R_global.app->client;
    RWM_SET_WINDOW_IMAGE(label, R_WINDOW_ACTIVE_STATE);
    Rwm_draw_text(label);
    if (!(wm->optflags & RWM_CLICK_MENU_FLAG)) {
        Rwm_show_pager(pager);
    }

    return;
}

void
Rwm_pager_leavenotify_handler(void *arg,
                              XEvent *event)
{
    struct R_window *pager;
    struct R_window *label;
    
    pager = arg;
    label = pager->chain;
    RWM_SET_WINDOW_IMAGE(label, R_WINDOW_NORMAL_STATE);
    Rwm_draw_text(label);
    Rwm_hide_pager(pager);

    return;
}

void
Rwm_pager_label_buttonpress_handler(void *arg,
                                    XEvent *event)
{
    struct R_window *window;
    struct R_window *pager;
    struct R_app    *app;
    struct R_wm     *wm;

    window = arg;
    app = window->app;
    wm = app->client;
    pager = wm->pager;
#if 0
    RWM_SET_WINDOW_IMAGE(window, R_WINDOW_CLICKED_STATE);
    Rwm_draw_text(window);
#endif
    if (event->xbutton.button == Button1
        && (wm->optflags & RWM_CLICK_MENU_FLAG)) {
        Rwm_show_pager(pager);
    }

    return;
}

void
Rwm_pager_label_buttonrelease_handler(void *arg,
                                    XEvent *event)
{
    struct R_window *window;

    window = arg;
#if 0
    RWM_SET_WINDOW_IMAGE(window, R_WINDOW_ACTIVE_STATE);
    Rwm_draw_text(window);
#endif

    return;
}

void
Rwm_pager_thumb_buttonpress_handler(void *arg,
                                    XEvent *event)
{
    struct R_window *pager;
    struct R_window *thumb;

    thumb = arg;
    pager = thumb->parent;
    Rwm_switch_desktop(thumb->app,
                       thumb->desktop);

    return;
}

void
Rwm_pager_click_func(void *arg,
                     XEvent *event,
                     void *bindarg)
{
    struct R_wm *wm;
    struct R_window *thumb;
    struct R_window *pager;
    struct R_window *label;

    thumb = arg;
    wm = thumb->app->client;
    pager = wm->pager;
    label = pager->chain;
    RWM_SET_WINDOW_IMAGE(label, R_WINDOW_NORMAL_STATE);
    Rwm_draw_text(label);
    Rwm_hide_pager(pager);
    Rwm_switch_desktop(thumb->app,
                       thumb->desktop);

    return;
}

