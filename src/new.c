/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"
#include "settings.h"

#define MAXIMUM_PROPERTY_LENGTH 4096

static void init_geometry(Client *c);
static void reparent(Client *c);
static void *get_property(Window w, Atom property, Atom req_type,
    unsigned long *nitems_return);
#ifdef XDEBUG
static const char *map_state_string(int map_state);
static const char *gravity_string(int gravity);
static void debug_wm_normal_hints(XSizeHints *size);
#else
# define debug_wm_normal_hints(s)
#endif

void make_new_client(Window w, ScreenInfo *s) {
  Client *c;
  char *name;
  long eventmask;

  XGrabServer(dpy);

  /* First a bit of interaction with the error handler due to X's
   * tendency to batch event notifications.  We set a global variable to
   * the id of the window we're initialising then do simple X call on
   * that window.  If an error is raised by this (and nothing else should
   * do so as we've grabbed the server), the error handler resets the
   * variable indicating the window has already disappeared, so we stop
   * trying to manage it. */
  initialising = w;
  XFetchName(dpy, w, &name);
  XSync(dpy, False);
  /* If 'initialising' is now set to None, that means doing the
   * XFetchName raised BadWindow - the window has been removed before
   * we got a chance to grab the server. */
  if (initialising == None) {
    LOG_DEBUG("make_new_client() : XError occurred for initialising window - aborting...\n");
    XUngrabServer(dpy);
    return;
  }
  initialising = None;
  LOG_DEBUG("make_new_client(): %s\n", name ? name : "Untitled");
  if (name)
    XFree(name);

  c = malloc(sizeof(Client));
  /* Don't crash the window manager, just fail the operation. */
  if (!c) {
    LOG_ERROR("out of memory in new_client; limping onward\n");
    return;
  }
  c->next = head_client;
  head_client = c;

  c->screen = s;
  c->window = w;
  c->ignore_unmap = 0;
  c->remove = 0;

  /* Ungrab the X server as soon as possible. Now that the client is
   * malloc()ed and attached to the list, it is safe for any subsequent
   * X calls to raise an X error and thus flag it for removal. */
  XUngrabServer(dpy);

  c->border = atoi( settings_get( "border.width" ) );

  init_geometry(c);

#ifdef DEBUG
  {
    Client *p;
    int i = 0;
    for (p = head_client; p; p = p->next)
      i++;
    LOG_DEBUG("make_new_client() : new window %dx%d+%d+%d, wincount=%d\n", c->width, c->height, c->x, c->y, i);
  }
#endif

  eventmask = PropertyChangeMask;
  eventmask |= ColormapChangeMask;
  if (atoi(settings_get("mouse.focus")))
      eventmask |= EnterWindowMask;

  XSelectInput(dpy, c->window,  eventmask);

  reparent(c);

  if (have_shape) {
      XShapeSelectInput(dpy, c->window, ShapeNotifyMask);
      set_shape(c);
  }

  /* Only map the window frame (and thus the window) if it's supposed
   * to be visible on this virtual desktop. */
  if (s->virtual_desktop == c->virtual_desktop)
  {
    unhide(c, RAISE);
    if (!atoi(settings_get("mouse.focus")))
        select_client(c);
  }
  else {
    set_wm_state(c, IconicState);
  }
  update_net_wm_desktop(c);
}

/* Calls XGetWindowAttributes, XGetWMHints and XGetWMNormalHints to determine
 * window's initial geometry.
 *
 * XGetWindowAttributes 
 */
static void init_geometry(Client *c) {
  long size_flags;
  XWindowAttributes attr;
  unsigned long nitems;
  PropMwmHints *mprop;
  unsigned long i;
  unsigned long *lprop;
  Atom *aprop;

  if ( (mprop = get_property(c->window, mwm_hints, mwm_hints, &nitems)) ) {
    if (nitems >= PROP_MWM_HINTS_ELEMENTS
        && (mprop->flags & MWM_HINTS_DECORATIONS)
        && !(mprop->decorations & MWM_DECOR_ALL)
        && !(mprop->decorations & MWM_DECOR_BORDER)) {
      c->border = 0;
    }
    XFree(mprop);
  }

  c->virtual_desktop = c->screen->virtual_desktop;
  if ( (lprop = get_property(c->window, xa_net_wm_desktop, XA_CARDINAL, &nitems)) ) {
    if (nitems && lprop[0] >=1 && lprop[0] <= 8)
      c->virtual_desktop = lprop[0];
    XFree(lprop);
  }
  remove_sticky(c);
  if ( (aprop = get_property(c->window, xa_net_wm_state, XA_ATOM, &nitems)) ) {
    for (i = 0; i < nitems; i++) {
      if (aprop[i] == xa_net_wm_state_sticky)
        add_sticky(c);
    }
    XFree(aprop);
  }

  /* Get current window attributes */
  LOG_XDEBUG("XGetWindowAttributes()\n");
  XGetWindowAttributes(dpy, c->window, &attr);
  LOG_XDEBUG("\t(%s) %dx%d+%d+%d, bw = %d\n", map_state_string(attr.map_state), attr.width, attr.height, attr.x, attr.y, attr.border_width);
  c->old_border = attr.border_width;
  c->oldw = c->oldh = 0;
  c->cmap = attr.colormap;

  size_flags = get_wm_normal_hints(c);

  if ((attr.width >= c->min_width) && (attr.height >= c->min_height)) {
  /* if (attr.map_state == IsViewable || (size_flags & (PSize | USSize))) { */
    c->width = attr.width;
    c->height = attr.height;
  } else {
    c->width = c->min_width;
    c->height = c->min_height;
    send_config(c);
  }
  if ((attr.map_state == IsViewable)
      || (size_flags & (/*PPosition |*/ USPosition))) {
    c->x = attr.x;
    c->y = attr.y;
  } else {
    int xmax = DisplayWidth(dpy, c->screen->screen);
    int ymax = DisplayHeight(dpy, c->screen->screen);
    int x, y;
    get_mouse_position(&x, &y, c->screen->root);
    c->x = (x * (xmax - c->border - c->width)) / xmax;
    c->y = (y * (ymax - c->border - c->height)) / ymax;
    send_config(c);
  }

  LOG_DEBUG("\twindow started as %dx%d +%d+%d\n", c->width, c->height, c->x, c->y);
  if (attr.map_state == IsViewable) {
    /* The reparent that is to come would trigger an unmap event */
    c->ignore_unmap++;
  }
  gravitate(c);
}

static void reparent(Client *c) {
  XSetWindowAttributes p_attr;
  XColor border_colour_inactive;
  XColor dummy;

  XAllocNamedColor(dpy, DefaultColormap(dpy, c->screen->screen), settings_get( "border.colour.inactive" ), &border_colour_inactive, &dummy);

  p_attr.border_pixel = border_colour_inactive.pixel;
  p_attr.override_redirect = True;
  p_attr.event_mask = ChildMask | ButtonPressMask;

  if (atoi(settings_get("mouse.focus")))
      p_attr.event_mask |= EnterWindowMask;

  c->parent = XCreateWindow(dpy, c->screen->root, c->x-c->border, c->y-c->border,
    c->width, c->height, c->border,
    DefaultDepth(dpy, c->screen->screen), CopyFromParent,
    DefaultVisual(dpy, c->screen->screen),
    CWOverrideRedirect | CWBorderPixel | CWEventMask, &p_attr);

  XAddToSaveSet(dpy, c->window);
  XSetWindowBorderWidth(dpy, c->window, 0);
  XReparentWindow(dpy, c->window, c->parent, 0, 0);
  XMapWindow(dpy, c->window);
  grab_button(c->parent, grabmask2, AnyButton);
}

/* Get WM_NORMAL_HINTS property */
long get_wm_normal_hints(Client *c) {
  XSizeHints *size;
  long flags;
  long dummy;
  size = XAllocSizeHints();
  LOG_XDEBUG("XGetWMNormalHints()\n");
  XGetWMNormalHints(dpy, c->window, size, &dummy);
  debug_wm_normal_hints(size);
  flags = size->flags;
  if (flags & PMinSize) {
    c->min_width = size->min_width;
    c->min_height = size->min_height;
  } else {
    c->min_width = c->min_height = 0;
  }
  if (flags & PMaxSize) {
    c->max_width = size->max_width;
    c->max_height = size->max_height;
  } else {
    c->max_width = c->max_height = 0;
  }
  if (flags & PBaseSize) {
    c->base_width = size->base_width;
    c->base_height = size->base_height;
  } else {
    c->base_width = c->min_width;
    c->base_height = c->min_height;
  }
  c->width_inc = c->height_inc = 1;
  if (flags & PResizeInc) {
    c->width_inc = size->width_inc ? size->width_inc : 1;
    c->height_inc = size->height_inc ? size->height_inc : 1;
  }
  if (!(flags & PMinSize)) {
    c->min_width = c->base_width + c->width_inc;
    c->min_height = c->base_height + c->height_inc;
  }
  if (flags & PWinGravity) {
    c->win_gravity = size->win_gravity;
  } else {
    c->win_gravity = NorthWestGravity;
  }
  XFree(size);
  return flags;
}

static void *get_property(Window w, Atom property, Atom req_type,
    unsigned long *nitems_return) {
  Atom actual_type;
  int actual_format;
  unsigned long bytes_after;
  unsigned char *prop;
  if (XGetWindowProperty(dpy, w, property,
        0L, MAXIMUM_PROPERTY_LENGTH / 4, False,
        req_type, &actual_type, &actual_format,
        nitems_return, &bytes_after, &prop)
      == Success) {
    if (actual_type == req_type)
      return (void *)prop;
    XFree(prop);
  }
  return NULL;
}

#ifdef XDEBUG
static const char *map_state_string(int map_state) {
  const char *map_states[4] = {
    "IsUnmapped",
    "IsUnviewable",
    "IsViewable",
    "Unknown"
  };
  return ((unsigned int)map_state < 3)
    ? map_states[map_state]
    : map_states[3];
}

static const char *gravity_string(int gravity) {
  const char *gravities[12] = {
    "ForgetGravity",
    "NorthWestGravity",
    "NorthGravity",
    "NorthEastGravity",
    "WestGravity",
    "CenterGravity",
    "EastGravity",
    "SouthWestGravity",
    "SouthGravity",
    "SouthEastGravity",
    "StaticGravity",
    "Unknown"
  };
  return ((unsigned int)gravity < 11) ? gravities[gravity] : gravities[11];
}

static void debug_wm_normal_hints(XSizeHints *size) {
  if (size->flags & 15) {
    LOG_XDEBUG("\t");
    if (size->flags & USPosition) {
      LOG_XDEBUG("USPosition ");
    }
    if (size->flags & USSize) {
      LOG_XDEBUG("USSize ");
    }
    if (size->flags & PPosition) {
      LOG_XDEBUG("PPosition ");
    }
    if (size->flags & PSize) {
      LOG_XDEBUG("PSize");
    }
    LOG_XDEBUG("\n");
  }
  if (size->flags & PMinSize) {
    LOG_XDEBUG("\tPMinSize: min_width = %d, min_height = %d\n", size->min_width, size->min_height);
  }
  if (size->flags & PMaxSize) {
    LOG_XDEBUG("\tPMaxSize: max_width = %d, max_height = %d\n", size->max_width, size->max_height);
  }
  if (size->flags & PResizeInc) {
    LOG_XDEBUG("\tPResizeInc: width_inc = %d, height_inc = %d\n",
        size->width_inc, size->height_inc);
  }
  if (size->flags & PAspect) {
    LOG_XDEBUG("\tPAspect: min_aspect = %d/%d, max_aspect = %d/%d\n",
        size->min_aspect.x, size->min_aspect.y,
        size->max_aspect.x, size->max_aspect.y);
  }
  if (size->flags & PBaseSize) {
    LOG_XDEBUG("\tPBaseSize: base_width = %d, base_height = %d\n",
        size->base_width, size->base_height);
  }
  if (size->flags & PWinGravity) {
    LOG_XDEBUG("\tPWinGravity: %s\n", gravity_string(size->win_gravity));
  }
}
#endif
