/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"
#include "settings.h"

static int send_xmessage(Window w, Atom a, long x);

/* used all over the place.  return the client that has specified window as
 * either window or parent */

Client *find_client(Window w) {
  Client *c;

  for (c = head_client; c; c = c->next)
      if (c->xstuff && ((w == c->xstuff->parent) || (w == c->xstuff->window)))
      return c;
  return NULL;
}

void set_wm_state(Client *c, int state) {
  /* Using "long" for the type of "data" looks wrong, but the
   * fine people in the X Consortium defined it this way
   * (even on 64-bit machines).
   */
  long data[2];
  data[0] = state;
  data[1] = None;
  XChangeProperty(dpy, c->xstuff->window, xa_wm_state, xa_wm_state, 32,
      PropModeReplace, (unsigned char *)data, 2);
}

void send_config(Client *c) {
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.event = c->xstuff->window;
  ce.window = c->xstuff->window;
  ce.x = c->x;
  ce.y = c->y;
  ce.width = c->width;
  ce.height = c->height;
  ce.border_width = 0;
  ce.above = None;
  ce.override_redirect = False;

  XSendEvent(dpy, c->xstuff->window, False, StructureNotifyMask, (XEvent *)&ce);
}

/* Support for 'gravitating' clients based on their original
 * border width and configured window manager frame width. */
void gravitate_client(Client *c, int sign) {
  int d0 = sign * c->border;
  int d1 = sign * c->old_border;
  int d2 = sign * (2*c->old_border - c->border);
  switch (c->win_gravity) {
    case NorthGravity:
      c->x += d1;
      c->y += d0;
      break;
    case NorthEastGravity:
      c->x += d2;
      c->y += d0;
      break;
    case EastGravity:
      c->x += d2;
      c->y += d1;
      break;
    case SouthEastGravity:
      c->x += d2;
      c->y += d2;
      break;
    case SouthGravity:
      c->x += d1;
      c->y += d2;
      break;
    case SouthWestGravity:
      c->x += d0;
      c->y += d2;
      break;
    case WestGravity:
      c->x += d0;
      c->y += d1;
      break;
    case NorthWestGravity:
    default:
      c->x += d0;
      c->y += d0;
      break;
  }
}

void select_client(Client *c) {
  XColor border_colour_active, border_colour_inactive;
  XColor border_colour_float_active, border_colour_float_inactive;
  XColor dummy;
  unsigned long bpixel;

  if (current)
  {
    XAllocNamedColor(dpy, DefaultColormap(dpy, current->xstuff->screen->screen), settings_get( "border.colour.inactive" ), &border_colour_inactive, &dummy);
    XAllocNamedColor(dpy, DefaultColormap(dpy, current->xstuff->screen->screen), settings_get( "border.colour.float.inactive" ), &border_colour_float_inactive, &dummy);
    if (is_sticky(current))
      bpixel = border_colour_float_inactive.pixel;
    else
      bpixel = border_colour_inactive.pixel;
    XSetWindowBorder(dpy, current->xstuff->parent, bpixel);
  }
  if (c) {
    XAllocNamedColor(dpy, DefaultColormap(dpy, c->xstuff->screen->screen), settings_get( "border.colour.active" ), &border_colour_active, &dummy);
    XAllocNamedColor(dpy, DefaultColormap(dpy, c->xstuff->screen->screen), settings_get( "border.colour.float.active" ), &border_colour_float_active, &dummy);
    if (is_sticky(c))
      bpixel = border_colour_float_active.pixel;
    else
      bpixel = border_colour_active.pixel;
    XSetWindowBorder(dpy, c->xstuff->parent, bpixel);
    XInstallColormap(dpy, c->xstuff->cmap);
    XSetInputFocus(dpy, c->xstuff->window, RevertToPointerRoot, CurrentTime);
  }
  current = c;
}

void float_client(Client *c) {
  toggle_sticky(c);
  select_client(c);
  update_net_wm_state(c);
}

void remove_client(Client *c) {
  Client *p;

  LOG_DEBUG("remove_client() : Removing...\n");

  if (!c || !c->xstuff) return;

  XGrabServer(dpy);
  ignore_xerror = 1;

  /* ICCCM 4.1.3.1
   * "When the window is withdrawn, the window manager will either
   *  change the state field's value to WithdrawnState or it will
   *  remove the WM_STATE property entirely."
   * EWMH 1.3
   * "The Window Manager should remove the property whenever a
   *  window is withdrawn but it should leave the property in
   *  place when it is shutting down." (both _NET_WM_DESKTOP and
   *  _NET_WM_STATE) */

  if (c->remove) {
    LOG_DEBUG("\tremove_client() : setting WithdrawnState\n");
    set_wm_state(c, WithdrawnState);
    XDeleteProperty(dpy, c->xstuff->window, xa_net_wm_desktop);
    XDeleteProperty(dpy, c->xstuff->window, xa_net_wm_state);
  }

  ungravitate(c);
  if (c->xstuff->screen)
      XReparentWindow(dpy, c->xstuff->window, c->xstuff->screen->root, c->x, c->y);
  XSetWindowBorderWidth(dpy, c->xstuff->window, c->old_border);
  XRemoveFromSaveSet(dpy, c->xstuff->window);
  if (c->xstuff->parent)
    XDestroyWindow(dpy, c->xstuff->parent);

  if (head_client == c) head_client = c->next;
  else for (p = head_client; p && p->next; p = p->next)
    if (p->next == c) p->next = c->next;

  if (current == c)
    current = NULL;  /* an enter event should set this up again */

  free(c->xstuff);
  free(c);
#ifdef DEBUG
  {
    Client *pp;
    int i = 0;
    for (pp = head_client; pp; pp = pp->next)
      i++;
    LOG_DEBUG("\tremove_client() : free(), window count now %d\n", i);
  }
#endif

  XUngrabServer(dpy);
  XFlush(dpy);
  ignore_xerror = 0;
  LOG_DEBUG("remove_client() returning\n");
}

void send_wm_delete(Client *c, int kill_client) {
  int i, n, found = 0;
  Atom *protocols;

  if (!kill_client && XGetWMProtocols(dpy, c->xstuff->window, &protocols, &n)) {
    for (i = 0; i < n; i++)
      if (protocols[i] == xa_wm_delete)
        found++;
    XFree(protocols);
  }
  if (found)
    send_xmessage(c->xstuff->window, xa_wm_protos, xa_wm_delete);
  else
    XKillClient(dpy, c->xstuff->window);
}

static int send_xmessage(Window w, Atom a, long x) {
  XEvent ev;

  ev.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = CurrentTime;

  return XSendEvent(dpy, w, False, NoEventMask, &ev);
}

void set_shape(Client *c) {
  int bounding_shaped;
  int i, b;  unsigned int u;  /* dummies */

  if (!have_shape) return;
  /* Logic to decide if we have a shaped window cribbed from fvwm-2.5.10.
   * Previous method (more than one rectangle returned from
   * XShapeGetRectangles) worked _most_ of the time. */
  if (XShapeQueryExtents(dpy, c->xstuff->window, &bounding_shaped, &i, &i,
        &u, &u, &b, &i, &i, &u, &u) && bounding_shaped) {
    LOG_DEBUG("%d shape extents\n", bounding_shaped);
    XShapeCombineShape(dpy, c->xstuff->parent, ShapeBounding, 0, 0,
        c->xstuff->window, ShapeBounding, ShapeSet);
  }
}
