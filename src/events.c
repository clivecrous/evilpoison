/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"
#include "settings.h"
#include "command.h"

#include <X11/cursorfont.h>

struct _ksconv {
    BindKeySymMask *chain;
    char * command;
};

static int num_keyconvs = 0;
static struct _ksconv *key_conversions = NULL;

void add_key_binding(KeySym k, unsigned int mask, char *cmd) {
  if (!cmd) return;

  key_conversions = realloc(key_conversions,
      sizeof(struct _ksconv) * (num_keyconvs+1));

  key_conversions[num_keyconvs].chain = malloc( sizeof( BindKeySymMask ) );
  key_conversions[num_keyconvs].chain->symbol = k;
  key_conversions[num_keyconvs].chain->mask = mask;
  key_conversions[num_keyconvs].command = malloc( strlen( cmd ) + 1 );
  strcpy( key_conversions[ num_keyconvs ].command, cmd );

  num_keyconvs++;
}

void free_key_bindings() {
  if (key_conversions) {
    // FIXME Inner allocations aren't freed yet.
	  free(key_conversions);
	  key_conversions = NULL;
	  num_keyconvs = 0;
  }
}

int find_key_binding(KeySym k, unsigned int mask) {
  int i;

  mask &= (0xFFFF-numlockmask);

  for (i = 0; i < num_keyconvs; i++)
    if ( k == key_conversions[i].chain->symbol && mask == key_conversions[i].chain->mask )
      return i;

  return -1;
}

int global_cmdmode = 0;
static void handle_key_event(XKeyEvent *e) {
  KeySym realkey = XKeycodeToKeysym(dpy,e->keycode,0);
  BindKeySymMask *prefix = keycode_convert( settings_get( "prefix" ) );

  if (!prefix) return;

  if ( realkey != prefix->symbol ||
       (e->state & prefix->mask ) != prefix->mask )
  {
    free( prefix );
    return;
  }
  free( prefix );

  int key_enum;
  XEvent ev;

  global_cmdmode = 0;
	Client *c;

  ScreenInfo *current_screen;

  c = current;
  current_screen = find_current_screen();

  if (XGrabKeyboard(dpy, e->root, GrabModeAsync, False, GrabModeAsync, CurrentTime) == GrabSuccess) {
    Cursor command_mode_cursor = XCreateFontCursor( dpy, XC_icon );
    XGrabPointer(dpy, e->root, False, 0, GrabModeAsync, GrabModeAsync, None, command_mode_cursor, CurrentTime );

    int modifier;
    do {
      modifier = 0;

      do {
          XMaskEvent(dpy, KeyPressMask|KeyReleaseMask, &ev);
      } while (ev.type != KeyPress);

      realkey = XKeycodeToKeysym(dpy, ev.xkey.keycode, 0);
      key_enum = find_key_binding(realkey, ev.xkey.state);

      if ( key_enum != -1 )
        command_execute( key_conversions[key_enum].command );
      else {
        switch ( realkey )
        {
          /* Ignore Modifiers */
          case XK_Shift_L:
          case XK_Shift_R:
          case XK_Control_L:
          case XK_Control_R:
          case XK_Caps_Lock:
          case XK_Shift_Lock:
          case XK_Meta_L:
          case XK_Meta_R:
          case XK_Alt_L:
          case XK_Alt_R:
          case XK_Super_L:
          case XK_Super_R:
          case XK_Hyper_L:
          case XK_Hyper_R:
            modifier = 1;
            break;
          default:
            global_cmdmode = 0;
            break;
        }
      }

    } while ( global_cmdmode || modifier );

		XUngrabKeyboard(dpy, CurrentTime);
		XUngrabPointer(dpy, CurrentTime);
  }

}

static void handle_button_event(XButtonEvent *e) {
	Client *c = find_client(e->window);

	if (c) {
		switch (e->button) {
			case Button1:
        command_execute( "window.move.mouse" );
        break;
			case Button2:
        command_execute( "window.lower" );
        break;
			case Button3:
        command_execute( "window.resize.mouse" );
        break;
			default: break;
		}
	}
  switch (e->button) {
    case Button4:
      command_execute( "window.next" );
      break;
    case Button5:
      command_execute( "window.previous" );
      break;
  }
}

static void handle_configure_request(XConfigureRequestEvent *e) {
	Client *c = find_client(e->window);
	XWindowChanges wc;
	unsigned int value_mask = e->value_mask;

	wc.sibling = e->above;
	wc.stack_mode = e->detail;
	wc.width = e->width;
	wc.height = e->height;
	if (c) {
		ungravitate(c);
		if (value_mask & CWWidth) c->width = e->width;
		if (value_mask & CWHeight) c->height = e->height;
		if (value_mask & CWX) c->x = e->x;
		if (value_mask & CWY) c->y = e->y;
		if (value_mask & CWStackMode && value_mask & CWSibling) {
			Client *sibling = find_client(e->above);
			if (sibling) {
				wc.sibling = sibling->parent;
			}
		}
		if (c->x == 0 && c->width >= DisplayWidth(dpy, c->screen->screen)) {
			c->x -= c->border;
		}
		if (c->y == 0 && c->height >= DisplayHeight(dpy, c->screen->screen)) {
			c->y -= c->border;
		}
		gravitate(c);

		wc.x = c->x - c->border;
		wc.y = c->y - c->border;
		wc.border_width = c->border;
		LOG_XDEBUG("XConfigureWindow(dpy, parent(%x), %lx, &wc);\n", (unsigned int)c->parent, value_mask);
		XConfigureWindow(dpy, c->parent, value_mask, &wc);
		XMoveResizeWindow(dpy, c->window, 0, 0, c->width, c->height);
		if ((value_mask & (CWX|CWY)) && !(value_mask & (CWWidth|CWHeight))) {
			send_config(c);
		}
		wc.border_width = 0;
	} else {
		wc.x = c ? 0 : e->x;
		wc.y = c ? 0 : e->y;
		LOG_XDEBUG("XConfigureWindow(dpy, window(%x), %lx, &wc);\n", (unsigned int)e->window, value_mask);
		XConfigureWindow(dpy, e->window, value_mask, &wc);
	}
}

static void handle_map_request(XMapRequestEvent *e) {
	Client *c = find_client(e->window);

	if (c) {
		if (c->vdesk != c->screen->vdesk)
			switch_vdesk(c->screen, c->vdesk);
		unhide(c, RAISE);
	} else {
		XWindowAttributes attr;
		LOG_DEBUG("handle_map_request() : don't know this window, calling make_new_client();\n");
		XGetWindowAttributes(dpy, e->window, &attr);
		make_new_client(e->window, find_screen(attr.root));
	}
}

static void handle_unmap_event(XUnmapEvent *e) {
	Client *c = find_client(e->window);

	LOG_DEBUG("handle_unmap_event():\n");
	if (c) {
		if (c->ignore_unmap) {
			c->ignore_unmap--;
			LOG_DEBUG("\tignored (%d ignores remaining)\n", c->ignore_unmap);
		} else {
			LOG_DEBUG("\tflagging client for removal\n");
			c->remove = 1;
			need_client_tidy = 1;
		}
	} else {
		LOG_DEBUG("\tunknown client!\n");
	}
}

static void handle_colormap_change(XColormapEvent *e) {
	Client *c = find_client(e->window);

	if (c && e->new) {
		c->cmap = e->colormap;
		XInstallColormap(dpy, c->cmap);
	}
}

static void handle_property_change(XPropertyEvent *e) {
	Client *c = find_client(e->window);

	if (c) {
		if (e->atom == XA_WM_NORMAL_HINTS) {
			get_wm_normal_hints(c);
		}
	}
}

static void handle_enter_event(XCrossingEvent *e) {
	Client *c;

	if (!atoi(settings_get("mouse.focus"))) return;

	if ((c = find_client(e->window))) {
		if (c->vdesk != c->screen->vdesk)
			return;
		select_client(c);
	}
}

static void handle_mappingnotify_event(XMappingEvent *e) {
	XRefreshKeyboardMapping(e);
	if (e->request == MappingKeyboard) {
		int i;
		for (i = 0; i < num_screens; i++) {
			grab_keys_for_screen(&screens[i]);
		}
	}
}

static void handle_shape_event(XShapeEvent *e) {
	Client *c = find_client(e->window);
	if (c)
		set_shape(c);
}

void event_main_loop(void) {
	XEvent ev;
	/* main event loop here */
	for (;;) {
		XNextEvent(dpy, &ev);
		switch (ev.type) {
			case KeyPress:
				handle_key_event(&ev.xkey); break;
			case ButtonPress:
				handle_button_event(&ev.xbutton); break;
			case ConfigureRequest:
				handle_configure_request(&ev.xconfigurerequest); break;
			case MapRequest:
				handle_map_request(&ev.xmaprequest); break;
			case ColormapNotify:
				handle_colormap_change(&ev.xcolormap); break;
			case EnterNotify:
				handle_enter_event(&ev.xcrossing); break;
			case PropertyNotify:
				handle_property_change(&ev.xproperty); break;
			case UnmapNotify:
				handle_unmap_event(&ev.xunmap); break;
			case MappingNotify:
				handle_mappingnotify_event(&ev.xmapping); break;
			default:
				if (have_shape && ev.type == shape_event) {
					handle_shape_event((XShapeEvent *)&ev);
				}
		}
		if (need_client_tidy) {
			Client *c, *nc;
			int donext = 0;
			for (c = head_client; c; c = nc) {
				nc = c->next;
				if (c->remove) {
				    if (c == current) donext = 1;
					remove_client(c);
				}
			}
			if (donext)
			    next();
		}
	}
}
