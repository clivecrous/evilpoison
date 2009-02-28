/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"
#include "settings.h"

#include "xinerama.h"

static Window create_info_window(Client *client);
static void update_info_window(Client *c, Window info_window);
static void grab_keysym(Window w, unsigned int mask, KeySym keysym);

void remove_text_window(Window window);
void remove_text_window(Window window) {
	if ( window ) XDestroyWindow(dpy, window);
	window = None;
}

void destroy_text_window(Window *window);
void destroy_text_window(Window *window)
{
  remove_text_window( *window );
  free( window );
  XFlush( dpy );
}

void *delayed_destroy_text_window(Window *window);
void *delayed_destroy_text_window(Window *window)
{
  usleep( 1000 * atoi( settings_get( "text.delay" ) ) );
  destroy_text_window( window );
  pthread_exit( NULL );
  return NULL;
}

void internal_echo( char *message )
{
  XColor text_colour_background;
  XColor text_border_colour;
  XColor dummy;

  ScreenInfo *current_screen = find_current_screen();
  if (!current_screen) return; // No screen to echo to (yet)

  XAllocNamedColor(dpy, DefaultColormap(dpy, current_screen->screen), settings_get( "text.colour.background" ), &text_colour_background, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, current_screen->screen), settings_get( "text.border.colour" ), &text_border_colour, &dummy);

  Window *echo_message_window = malloc( sizeof( Window ) );
  *echo_message_window = XCreateSimpleWindow(dpy, current_screen->root, -4, -4, 2, 2,
			atoi( settings_get( "text.border.width" ) ), text_border_colour.pixel, text_colour_background.pixel);

  if (!*echo_message_window) return;

	XMapRaised(dpy, *echo_message_window);

  XFontStruct * font = XLoadQueryFont(dpy, settings_get( "text.font" ) );
  if (!font) return;

	int window_width = XTextWidth( font, message, strlen(message)) + 8;
	int window_height = font->max_bounds.ascent + font->max_bounds.descent;

  XMoveResizeWindow(dpy, *echo_message_window, 0, 0, window_width, window_height);

	XClearWindow(dpy, *echo_message_window);

  XGCValues gv;
  GC gc;
  XColor text_colour_foreground;

  XAllocNamedColor(dpy, DefaultColormap(dpy, current_screen->screen), settings_get( "text.colour.foreground" ), &text_colour_foreground, &dummy);

	gv.function = GXcopy;
	gv.subwindow_mode = IncludeInferiors;
	gv.line_width = atoi( settings_get( "border.width" ) );
	gv.font = font->fid;
  gv.foreground = text_colour_foreground.pixel;

  gc = XCreateGC(dpy, current_screen->root, GCFunction | GCSubwindowMode | GCLineWidth | GCFont | GCForeground, &gv);

	XDrawString(dpy, *echo_message_window, gc, 4, window_height - 2,
			message, strlen(message));

  XFreeGC( dpy, gc );
  XFreeFont( dpy, font );

  XFlush(dpy);

  pthread_t thread;
  if ( pthread_create( &thread, NULL, (void *)delayed_destroy_text_window, (void *)echo_message_window ) != 0 )
    destroy_text_window( echo_message_window );
}

static Window create_info_window(Client *client) {
  XColor text_colour_background;
  XColor text_border_colour;
  XColor dummy;

  XAllocNamedColor(dpy, DefaultColormap(dpy, client->screen->screen), settings_get( "text.colour.background" ), &text_colour_background, &dummy);
  XAllocNamedColor(dpy, DefaultColormap(dpy, client->screen->screen), settings_get( "text.border.colour" ), &text_border_colour, &dummy);

  Window info_window = None;
	info_window = XCreateSimpleWindow(dpy, client->screen->root, -4, -4, 2, 2,
			atoi( settings_get( "text.border.width" ) ), text_border_colour.pixel, text_colour_background.pixel);
	XMapRaised(dpy, info_window);
	update_info_window(client, info_window);
  return info_window;
}

static void update_info_window(Client *client, Window info_window) {
	char *name;
	char buf[27];
	int namew, iwinx, iwiny, iwinw, iwinh, iwinb;
	int width_inc = client->width_inc, height_inc = client->height_inc;
  XFontStruct *font;

	if (!info_window) return;
	font = XLoadQueryFont(dpy, settings_get( "text.font" ) );
  if (!font) return;
  
	snprintf(buf, sizeof(buf), "%dx%d+%d+%d", (client->width-client->base_width)/width_inc,
		(client->height-client->base_height)/height_inc, client->x, client->y);
	iwinw = XTextWidth(font, buf, strlen(buf)) + 2;
	iwinh = font->max_bounds.ascent + font->max_bounds.descent;
	XFetchName(dpy, client->window, &name);
	if (name) {
		namew = XTextWidth(font, name, strlen(name));
		if (namew > iwinw)
			iwinw = namew + 2;
		iwinh = iwinh * 2;
	}
  iwinb = atoi( settings_get( "text.border.width" ) )*2;
	iwinx = client->x + client->width + client->border - ( iwinw + iwinb );
	iwiny = client->y - client->border;
	if (iwinx + iwinw + iwinb > DisplayWidth(dpy, client->screen->screen))
		iwinx = DisplayWidth(dpy, client->screen->screen) - (iwinw + iwinb);
	if (iwinx < 0)
		iwinx = 0;
	if (iwiny + iwinh > DisplayHeight(dpy, client->screen->screen))
		iwiny = DisplayHeight(dpy, client->screen->screen) - iwinh;
	if (iwiny < 0)
		iwiny = 0;
	XMoveResizeWindow(dpy, info_window, iwinx, iwiny, iwinw, iwinh);
	XClearWindow(dpy, info_window);

  XGCValues gv;
  GC gc;
  XColor text_colour_foreground, dummy;

  XAllocNamedColor(dpy, DefaultColormap(dpy, client->screen->screen), settings_get( "text.colour.foreground" ), &text_colour_foreground, &dummy);

	gv.function = GXcopy;
	gv.subwindow_mode = IncludeInferiors;
	gv.line_width = atoi( settings_get( "border.width" ) );
	gv.font = font->fid;
  gv.foreground = text_colour_foreground.pixel;

  gc = XCreateGC(dpy, client->screen->root, GCFunction | GCSubwindowMode | GCLineWidth | GCFont | GCForeground, &gv);

	if (name) {
		XDrawString(dpy, info_window, gc,
				1, iwinh / 2 - 1, name, strlen(name));
		XFree(name);
	}
	XDrawString(dpy, info_window, gc, 1, iwinh - 1,
			buf, strlen(buf));

  XFreeGC( dpy, gc );
  XFreeFont( dpy, font );
}

static void draw_outline(Client *c) {
  XGCValues gv;
  GC gc;

	gv.function = GXinvert;
	gv.subwindow_mode = IncludeInferiors;
	gv.line_width = atoi( settings_get( "border.width" ) );

  gc = XCreateGC(dpy, c->screen->root, GCFunction | GCSubwindowMode | GCLineWidth, &gv);

	XDrawRectangle(dpy, c->screen->root, gc,
		c->x - c->border, c->y - c->border,
		c->width + c->border, c->height + c->border);

  XFreeGC( dpy, gc );
}

static void recalculate_sweep(Client *c, int x1, int y1, int x2, int y2) {
	c->width = abs(x1 - x2);
	c->height = abs(y1 - y2);
	c->width -= (c->width - c->base_width) % c->width_inc;
	c->height -= (c->height - c->base_height) % c->height_inc;
	if (c->min_width && c->width < c->min_width) c->width = c->min_width;
	if (c->min_height && c->height < c->min_height) c->height = c->min_height;
	if (c->max_width && c->width > c->max_width) c->width = c->max_width;
	if (c->max_height && c->height > c->max_height) c->height = c->max_height;
	c->x = (x1 <= x2) ? x1 : x1 - c->width;
	c->y = (y1 <= y2) ? y1 : y1 - c->height;
}

void sweep(Client *c) {
	XEvent ev;
	int old_cx = c->x;
	int old_cy = c->y;

	if (!grab_pointer(c->screen->root, MouseMask, resize_curs)) return;

	XRaiseWindow(dpy, c->parent);
	Window info_window = create_info_window(c);
	XGrabServer(dpy);
	draw_outline(c);

	setmouse(c->window, c->width, c->height);
	for (;;) {
		XMaskEvent(dpy, MouseMask, &ev);
		switch (ev.type) {
			case MotionNotify:
        while (XCheckTypedEvent(dpy, MotionNotify, &ev));
				draw_outline(c); /* clear */
				XUngrabServer(dpy);
				recalculate_sweep(c, old_cx, old_cy, ev.xmotion.x, ev.xmotion.y);
				update_info_window(c,info_window);
				XSync(dpy, False);
				XGrabServer(dpy);
				draw_outline(c);
				break;
			case ButtonRelease:
				draw_outline(c); /* clear */
				XUngrabServer(dpy);
				remove_text_window(info_window);
				XUngrabPointer(dpy, CurrentTime);
				moveresize(c);
				return;
			default: break;
		}
	}
}

void show_info(Client *client) {

  if (!atoi( settings_get( "text.delay" ) )) return;

  Window *info_window = malloc( sizeof( Window ) );
	*info_window = create_info_window(client);
  XFlush(dpy);
  pthread_t thread;
  if ( pthread_create( &thread, NULL, (void *)delayed_destroy_text_window, (void *)info_window ) != 0 )
    destroy_text_window( info_window );
}

static int absmin(int a, int b) {
	if (abs(a) < abs(b))
		return a;
	return b;
}

static void snap_client(Client *c) {
	int dx, dy;
	Client *ci;

  int screen_origin_x = xinerama_screen_origin_x();
  int screen_origin_y = xinerama_screen_origin_y();

  int screen_width = xinerama_screen_width();
  int screen_height = xinerama_screen_height();

  int border_snap = atoi( settings_get( "border.snap" ) );

	/* snap to screen left border */
	if (abs(c->x - (screen_origin_x + c->border) ) < border_snap)
    c->x = screen_origin_x + c->border;
	/* snap to screen top border */
	if (abs(c->y - (screen_origin_y + c->border) ) < border_snap)
    c->y = screen_origin_y + c->border;
	/* snap to screen right border */
	if ( abs( (c->x + c->width + c->border) - (screen_origin_x + screen_width) ) < border_snap )
		c->x = screen_origin_x + screen_width - (c->width + c->border);
	/* snap to screen bottom border */
	if ( abs( (c->y + c->height + c->border) - (screen_origin_y + screen_height) ) < border_snap)
		c->y = screen_origin_y + screen_height - (c->height + c->border);

	/* snap to other windows */
	dx = dy = border_snap;
	for (ci = head_client; ci; ci = ci->next) {
		if (ci != c
				&& (ci->screen == c->screen)
				&& (ci->virtual_desktop == c->virtual_desktop)
				) {
			if (ci->y - ci->border - c->border - c->height - c->y <= border_snap && c->y - c->border - ci->border - ci->height - ci->y <= border_snap) {
				dx = absmin(dx, ci->x + ci->width - c->x + c->border + ci->border);
				dx = absmin(dx, ci->x + ci->width - c->x - c->width);
				dx = absmin(dx, ci->x - c->x - c->width - c->border - ci->border);
				dx = absmin(dx, ci->x - c->x);
			}
			if (ci->x - ci->border - c->border - c->width - c->x <= border_snap && c->x - c->border - ci->border - ci->width - ci->x <= border_snap) {
				dy = absmin(dy, ci->y + ci->height - c->y + c->border + ci->border);
				dy = absmin(dy, ci->y + ci->height - c->y - c->height);
				dy = absmin(dy, ci->y - c->y - c->height - c->border - ci->border);
				dy = absmin(dy, ci->y - c->y);
			}
		}
	}
	if (abs(dx) < border_snap)
		c->x += dx;
	if (abs(dy) < border_snap)
		c->y += dy;
	if (abs(c->x) == c->border && c->width == screen_width)
		c->x = 0;
	if (abs(c->y) == c->border && c->height == screen_height)
		c->y = 0;
}

void drag(Client *c) {
	XEvent ev;
	int x1, y1;
	int old_cx = c->x;
	int old_cy = c->y;

  int move_display = atoi( settings_get( "window.move.display" ) );

	if (!grab_pointer(c->screen->root, MouseMask, move_curs)) return;
	XRaiseWindow(dpy, c->parent);
	get_mouse_position(&x1, &y1, c->screen->root);
	Window info_window = create_info_window(c);
	if (!move_display) {
		XGrabServer(dpy);
		draw_outline(c);
	}
	for (;;) {
		XMaskEvent(dpy, MouseMask, &ev);
		switch (ev.type) {
			case MotionNotify:
        while (XCheckTypedEvent(dpy, MotionNotify, &ev));
				if (!move_display) {
					draw_outline(c); /* clear */
					XUngrabServer(dpy);
				}
				c->x = old_cx + (ev.xmotion.x - x1);
				c->y = old_cy + (ev.xmotion.y - y1);
				if ( atoi( settings_get( "border.snap" ) ) )
					snap_client(c);

				update_info_window(c,info_window);
				if (!move_display) {
					XSync(dpy, False);
					XGrabServer(dpy);
					draw_outline(c);
				} else {
					XMoveWindow(dpy, c->parent,
							c->x - c->border,
							c->y - c->border);
					send_config(c);
				}
				break;
			case ButtonRelease:
				if (!move_display) {
					draw_outline(c); /* clear */
					XUngrabServer(dpy);
				}
				remove_text_window(info_window);
				XUngrabPointer(dpy, CurrentTime);
				if (!move_display) {
					moveresize(c);
				}
				return;
			default: break;
		}
	}
}

/** Check whether the given client is maximised or not.
 * \param client The client window to be checked.
 * \param hv Check horizontal, vertical or both.
 * \return A boolean indicating if the client is maximised.
 */
static int client_maximised( Client *client, int hv )
{ return hv & MAXIMISE_HORZ ? (client->x == xinerama_screen_origin_x() + client->border) && (client->width == xinerama_screen_width() - client->border * 2) : 1 &&
         hv & MAXIMISE_VERT ? (client->y == xinerama_screen_origin_y() + client->border) && (client->height == xinerama_screen_height() - client->border * 2) : 1; }

void moveresize(Client *c) {
	XRaiseWindow(dpy, c->parent);
	XMoveResizeWindow(dpy, c->parent, c->x - c->border, c->y - c->border,
			c->width, c->height);
	XMoveResizeWindow(dpy, c->window, 0, 0, c->width, c->height);
	send_config(c);
}

void maximise_client(Client *c, int hv) {
  int maximised = client_maximised( c, hv );
	if (hv & MAXIMISE_HORZ) {
		if ( maximised ) {
			c->x = c->oldx;
			c->width = c->oldw;
		} else {
			c->oldx = c->x;
			c->oldw = c->width;
			c->x = xinerama_screen_origin_x() + c->border;
			c->width = xinerama_screen_width() - c->border * 2;
		}
	}
	if (hv & MAXIMISE_VERT) {
		if ( maximised ) {
			c->y = c->oldy;
			c->height = c->oldh;
		} else {
			c->oldy = c->y;
			c->oldh = c->height;
			c->y = xinerama_screen_origin_y() + c->border;
			c->height = xinerama_screen_height() - c->border * 2;
		}
	}
	moveresize(c);
	discard_enter_events();
}

void hide(Client *c) {
	/* This will generate an unmap event.  Tell event handler
	 * to ignore it. */
	c->ignore_unmap++;
	LOG_XDEBUG("screen:XUnmapWindow(parent);\n");
	XUnmapWindow(dpy, c->parent);
	set_wm_state(c, IconicState);
}

void unhide(Client *c, int raise_win) {
	raise_win ? XMapRaised(dpy, c->parent) : XMapWindow(dpy, c->parent);
	set_wm_state(c, NormalState);
}

static void nextprevious( Client *change_to )
{
  if (!change_to) return;
  if (change_to == current) return;

  // Disallow changing across virtual_desktops.
  ScreenInfo *current_screen = find_current_screen();
  if ( !current_screen || change_to->virtual_desktop != current_screen->virtual_desktop ) return;

	unhide(change_to, RAISE);
	select_client(change_to);

	if ( atoi( settings_get( "mouse.warp" ) ) ) {
	    setmouse(change_to->window, 0, 0);
	    setmouse(change_to->window, change_to->width + change_to->border - 1,
			change_to->height + change_to->border - 1);
	}

	discard_enter_events();
}

void next(void)
{
	Client *newc = current;
  ScreenInfo *current_screen = find_current_screen();

  if (newc) newc = newc->next;
  while ( newc && newc->virtual_desktop != current_screen->virtual_desktop )
    newc = newc->next;
  if (!newc) newc = head_client;
  while ( newc && newc->virtual_desktop != current_screen->virtual_desktop )
    newc = newc->next;

  if (newc && newc->virtual_desktop != current_screen->virtual_desktop)
    newc = NULL;

  if (newc) nextprevious( newc );
}

void previous(void)
{
  // TODO This will not wrap around correctly when foreign virtual_desktop clients are
  // involved.
	Client *newc = head_client;

  while (newc && newc->next != current ) newc = newc->next;

  if (!newc)
    for (newc = head_client; newc && newc->next; newc = newc->next );

  nextprevious( newc );
}

void switch_virtual_desktop(ScreenInfo *screen, int virtual_desktop_wanted) {

  /* Don't bother if nothing actually changes */
	if (virtual_desktop_wanted == screen->virtual_desktop) return;

	LOG_DEBUG("switch_virtual_desktop(): Switching screen %d's desktop to desktop %d\n", screen->screen, virtual_desktop_wanted);

  /* Deselect current window unless it's sticky */
	if (current && !is_sticky(current)) {
		select_client(NULL);
	}

	for ( Client * client_iterator = head_client;
        client_iterator;
        client_iterator = client_iterator->next )
  {

    /* Skip this windows on other screens  */
		if ( client_iterator->screen != screen ) continue;

    /* Move sticky windows to the new virtual desktop */
		if ( is_sticky(client_iterator) &&
         client_iterator->virtual_desktop != virtual_desktop_wanted)
    {
			client_iterator->virtual_desktop = virtual_desktop_wanted;
			update_net_wm_desktop(client_iterator);
		}

		if ( client_iterator->virtual_desktop == virtual_desktop_wanted )
    {
			unhide(client_iterator, NO_RAISE);
    }
    else
    {
			hide(client_iterator);
		}

	}

  /* Save current to allow history.back and switch */
	screen->other_virtual_desktop = screen->virtual_desktop;
	screen->virtual_desktop = virtual_desktop_wanted;

  /* Ensure that we have a window active on the new virtual desktop */
  if ( !current || current->virtual_desktop != virtual_desktop_wanted ) next();
}

ScreenInfo *find_screen(Window root) {
	int i;
	for (i = 0; i < num_screens; i++) {
		if (screens[i].root == root)
			return &screens[i];
	}
	return NULL;
}

ScreenInfo *find_current_screen(void) {
	Window cur_root, dw;
	int di;
	unsigned int dui;

  if (!screens) return 0;

	/* XQueryPointer is useful for getting the current pointer root */
	XQueryPointer(dpy, screens[0].root, &cur_root, &dw, &di, &di, &di, &di, &dui);
	return find_screen(cur_root);
}

static void grab_keysym(Window w, unsigned int mask, KeySym keysym) {
	KeyCode keycode = XKeysymToKeycode(dpy, keysym);
	XGrabKey(dpy, keycode, mask, w, True,
			GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, keycode, mask|LockMask, w, True,
			GrabModeAsync, GrabModeAsync);
	if (numlockmask) {
		XGrabKey(dpy, keycode, mask|numlockmask, w, True,
				GrabModeAsync, GrabModeAsync);
		XGrabKey(dpy, keycode, mask|numlockmask|LockMask, w, True,
				GrabModeAsync, GrabModeAsync);
	}
}

void grab_keys_for_screen(ScreenInfo *s) {
	/* Release any previous grabs */
	XUngrabKey(dpy, AnyKey, AnyModifier, s->root);
	/* We're only interested in the prefix key */
  BindKeySymMask *prefix = keycode_convert( settings_get( "prefix" ) );
	if (prefix) grab_keysym(s->root, prefix->mask, prefix->symbol );
  free( prefix );
}
