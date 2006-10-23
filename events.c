/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <stdlib.h>
#include "evilwm.h"
#include "log.h"

static void current_to_head(void) {
	Client *c;
	if (current && current != head_client) {
		for (c = head_client; c; c = c->next) {
			if (c->next == current) {
				c->next = current->next;
				current->next = head_client;
				head_client = current;
				break;
			}
		}
	}
}

struct _ksconv {
    KeySym kfrom;
    unsigned int kmask;
    int kto;
} const key_conversions[] = {
    {PREFIX_KEY, PREFIX_MOD,	KEY_PREFIX},
    {XK_m, 0,		KEY_CMDMODE},
    {XK_Return, 0,	KEY_NEXT},
    {XK_Tab, 0,		KEY_NEXT},
    {XK_c, 0,		KEY_NEW},
    {XK_y, 0,		KEY_TOPLEFT},
    {XK_u, 0,		KEY_TOPRIGHT},
    {XK_b, 0,		KEY_BOTTOMLEFT},
    {XK_n, 0,		KEY_BOTTOMRIGHT},
    {XK_h, 0,		KEY_LEFT},
    {XK_l, 0,		KEY_RIGHT},
    {XK_j, 0,		KEY_DOWN},
    {XK_k, 0,		KEY_UP},
    {XK_Left, 0,	KEY_LEFT},
    {XK_Right, 0,	KEY_RIGHT},
    {XK_Up, 0,		KEY_UP},
    {XK_Down, 0,	KEY_DOWN},

    {XK_a, 0,	KEY_RESIZELEFT},
    {XK_d, 0,	KEY_RESIZERIGHT},
    {XK_w, 0,		KEY_RESIZEUP},
    {XK_s, 0,	KEY_RESIZEDOWN},

    {XK_comma, 0, KEY_MOUSESWEEP},
    {XK_period, 0, KEY_MOUSEDRAG},

    {XK_Insert, 0,	KEY_LOWER},
    /*{XK_i, 0,		KEY_INFO},*/ /* FIXME: info causes wm to hang */

    {XK_plus, 0,	KEY_MAXVERT},
    {XK_minus, 0,	KEY_MAXHORIZ},
    {XK_x, 0,		KEY_MAX},
    {XK_v, 0,		KEY_VSPLIT},
    {XK_g, 0,		KEY_HSPLIT},


    {XK_Escape, 0,	KEY_KILL},
#ifdef VWM
    {XK_f, 0,		KEY_FIX},
    {XK_o, 0,	KEY_PREVDESK},
    {XK_p, 0,	KEY_NEXTDESK},
    {XK_1, 0,		KEY_DESK1},
    {XK_2, 0,		KEY_DESK2},
    {XK_3, 0,		KEY_DESK3},
    {XK_4, 0,		KEY_DESK4},
    {XK_5, 0,		KEY_DESK5},
    {XK_6, 0,		KEY_DESK6},
    {XK_7, 0,		KEY_DESK7},
    {XK_8, 0,		KEY_DESK8},
#endif
    {0,0,0}
};

#define ARRSIZE(x) (int)(sizeof(x) / sizeof(x[0]))

static int do_key_conversion(KeySym k, unsigned int mask) {
    int i;

    for (i = 0; i < ARRSIZE(key_conversions); i++)
	if (k == key_conversions[i].kfrom &&
	    ((key_conversions[i].kmask == 0) || (mask == key_conversions[i].kmask)))
	    return key_conversions[i].kto;

    return KEY_NONE;
}

static void move_client(Client *c) {
    moveresize(c);
    setmouse(c->window, c->width + c->border - 1, c->height + c->border - 1);
    discard_enter_events();
}

static void handle_key_event(XKeyEvent *e) {
    int key = do_key_conversion(XKeycodeToKeysym(dpy,e->keycode,0), e->state);
    KeySym realkey = e->keycode;
	Client *c;
	int width_inc, height_inc;
#ifdef VWM
	ScreenInfo *current_screen = find_current_screen();
#endif
	int cmdmode = 0;

	c = current;

	if (c) {
	    width_inc = (c->width_inc > 1) ? c->width_inc : 16;
	    height_inc = (c->height_inc > 1) ? c->height_inc : 16;
	}

	if ((key == KEY_PREFIX) && (e->type == KeyPress)) {
	    if (XGrabKeyboard(dpy, e->root, False, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess) {
		XEvent ev;

		do {
		    XMaskEvent(dpy, KeyPressMask|KeyReleaseMask, &ev);
		} while (ev.type != KeyPress);

		XUngrabKeyboard(dpy, CurrentTime);
		key = do_key_conversion(XKeycodeToKeysym(dpy, ev.xkey.keycode, 0), ev.xkey.state);
		realkey = ev.xkey.keycode;
	    } else key = KEY_NONE;
	} else key = KEY_NONE;

	do {

	    if (cmdmode) {
		c = current;

		if (XGrabKeyboard(dpy, e->root, False, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess) {
		    XEvent ev;
		    do {
			XMaskEvent(dpy, KeyPressMask|KeyReleaseMask, &ev);
		    } while (ev.type != KeyPress);
		    key = do_key_conversion(XKeycodeToKeysym(dpy,ev.xkey.keycode,0), ev.xkey.state);
		}
	    }

	switch(key) {
	case KEY_MOUSEDRAG:
	    if (c) drag(c);
	    break;
	case KEY_MOUSESWEEP:
	    if (c) sweep(c);
	    break;
		case KEY_LEFT:
		    if (c) {
			c->x -= 16;
			move_client(c);
		    }
		    break;
		case KEY_DOWN:
		    if (c) {
			c->y += 16;
			move_client(c);
		    }
		    break;
		case KEY_UP:
		    if (c) {
			c->y -= 16;
			move_client(c);
		    }
		    break;
		case KEY_RIGHT:
		    if (c) {
			c->x += 16;
			move_client(c);
		    }
		    break;
		case KEY_TOPLEFT:
		    if (c) {
			c->x = c->border;
			c->y = c->border;
			move_client(c);
		    }
		    break;
		case KEY_TOPRIGHT:
		    if (c) {
			c->x = DisplayWidth(dpy, c->screen->screen) - c->width-c->border;
			c->y = c->border;
			move_client(c);
		    }
		    break;
		case KEY_BOTTOMLEFT:
		    if (c) {
			c->x = c->border;
			c->y = DisplayHeight(dpy, c->screen->screen) - c->height-c->border;
			move_client(c);
		    }
		    break;
		case KEY_BOTTOMRIGHT:
		    if (c) {
			c->x = DisplayWidth(dpy, c->screen->screen) - c->width-c->border;
			c->y = DisplayHeight(dpy, c->screen->screen) - c->height-c->border;
			move_client(c);
		    }
		    break;
		case KEY_RESIZELEFT:
		    if (c) {
			c->width -= 16;
			move_client(c);
		    }
		    break;
		case KEY_RESIZERIGHT:
		    if (c) {
			c->width += 16;
			move_client(c);
		    }
		    break;
		case KEY_RESIZEUP:
		    if (c) {
			c->height -= 16;
			move_client(c);
		    }
		    break;
		case KEY_RESIZEDOWN:
		    if (c) {
			c->height += 16;
			move_client(c);
		    }
		    break;

		case KEY_NEW:
			spawn(opt_term);
			break;
		case KEY_NEXT:
			next();
			/* current_to_head();*/
			break;
		case KEY_KILL:
		    if (c) send_wm_delete(c, e->state & altmask);
			break;
		case KEY_LOWER:
		    if (c) XLowerWindow(dpy, c->parent);
			break;
		case KEY_INFO:
		    if (c) show_info(c, realkey);
			break;
		case KEY_MAX:
		    if (c) maximise_client(c, MAXIMISE_HORZ|MAXIMISE_VERT);
			break;
		case KEY_MAXVERT:
		    if (c) maximise_client(c, MAXIMISE_VERT);
			break;
		case KEY_MAXHORIZ:
		    if (c) maximise_client(c, MAXIMISE_HORZ);
			break;
	case KEY_VSPLIT:
	    if (c) {
		c->width = c->width / 2;
		move_client(c);
	    }
	    break;
	case KEY_HSPLIT:
	    if (c) {
		c->height = c->height / 2;
		move_client(c);
	    }
	    break;
#ifdef VWM
		case KEY_FIX:
		    if (c) fix_client(c);
			break;
		case KEY_DESK1: case KEY_DESK2: case KEY_DESK3: case KEY_DESK4:
		case KEY_DESK5: case KEY_DESK6: case KEY_DESK7: case KEY_DESK8:
			switch_vdesk(current_screen, KEY_TO_VDESK(key));
			break;
		case KEY_PREVDESK:
			if (current_screen->vdesk > KEY_TO_VDESK(KEY_DESK1)) {
				switch_vdesk(current_screen,
						current_screen->vdesk - 1);
			}
			break;
		case KEY_NEXTDESK:
			if (current_screen->vdesk < KEY_TO_VDESK(KEY_DESK8)) {
				switch_vdesk(current_screen,
						current_screen->vdesk + 1);
			}
			break;
#endif
	case KEY_CMDMODE:
	    if (cmdmode)
		XUngrabKeyboard(dpy, CurrentTime);
	    cmdmode = !cmdmode;
	    break;
	default:
	    if (cmdmode)
		XUngrabKeyboard(dpy, CurrentTime);
	    cmdmode = 0;
	    break;
	}

	} while (cmdmode);

}

#ifdef MOUSE
static void handle_button_event(XButtonEvent *e) {
	Client *c = find_client(e->window);

	if (c) {
		switch (e->button) {
			case Button1:
				drag(c); break;
			case Button2:
				sweep(c); break;
			case Button3:
				XLowerWindow(dpy, c->parent); break;
			default: break;
		}
	}
}
#endif

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
#ifdef VWM
		if (c->vdesk != c->screen->vdesk)
			switch_vdesk(c->screen, c->vdesk);
#endif
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

	LOG_DEBUG("handle_unmap_event(): ");
	if (c) {
		if (c->ignore_unmap) {
			c->ignore_unmap--;
			LOG_DEBUG("ignored (%d ignores remaining)\n", c->ignore_unmap);
		} else {
			LOG_DEBUG("flagging client for removal\n");
			c->remove = 1;
			need_client_tidy = 1;
		}
	} else {
		LOG_DEBUG("unknown client!\n");
	}
}

#ifdef COLOURMAP
static void handle_colormap_change(XColormapEvent *e) {
	Client *c = find_client(e->window);

	if (c && e->new) {
		c->cmap = e->colormap;
		XInstallColormap(dpy, c->cmap);
	}
}
#endif

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

	if ((c = find_client(e->window))) {
#ifdef VWM
		if (c->vdesk != c->screen->vdesk)
			return;
#endif
		select_client(c);
		current_to_head();
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

#ifdef SHAPE
static void handle_shape_event(XShapeEvent *e) {
	Client *c = find_client(e->window);
	if (c)
		set_shape(c);
}
#endif

void event_main_loop(void) {
	XEvent ev;
	/* main event loop here */
	for (;;) {
		XNextEvent(dpy, &ev);
		switch (ev.type) {
			case KeyPress:
				handle_key_event(&ev.xkey); break;
#ifdef MOUSE
			case ButtonPress:
				handle_button_event(&ev.xbutton); break;
#endif
			case ConfigureRequest:
				handle_configure_request(&ev.xconfigurerequest); break;
			case MapRequest:
				handle_map_request(&ev.xmaprequest); break;
#ifdef COLOURMAP
			case ColormapNotify:
				handle_colormap_change(&ev.xcolormap); break;
#endif
			case EnterNotify:
				handle_enter_event(&ev.xcrossing); break;
			case PropertyNotify:
				handle_property_change(&ev.xproperty); break;
			case UnmapNotify:
				handle_unmap_event(&ev.xunmap); break;
			case MappingNotify:
				handle_mappingnotify_event(&ev.xmapping); break;
#ifdef SHAPE
			default:
				if (have_shape && ev.type == shape_event) {
					handle_shape_event((XShapeEvent *)&ev);
				}
#endif
		}
		if (need_client_tidy) {
			Client *c, *nc;
			for (c = head_client; c; c = nc) {
				nc = c->next;
				if (c->remove)
					remove_client(c);
			}
		}
	}
}
