/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"

#include "command.h"
#include "settings.h"
#include "evilpoison_commands.h"
#include "commandline.h"

/* Commonly used X information */
Display     *dpy;
Cursor      move_curs;
Cursor      resize_curs;
int         num_screens;
ScreenInfo  *screens;
int         have_shape, shape_event;

/* Standard X protocol atoms */
Atom xa_wm_state;
Atom xa_wm_protos;
Atom xa_wm_delete;
Atom xa_wm_cmapwins;
/* Motif atoms */
Atom mwm_hints;
/* EWMH atoms */
Atom xa_net_wm_desktop;
Atom xa_net_wm_state;
Atom xa_net_wm_state_sticky;

/* Things that affect user interaction */
unsigned int numlockmask = 0;
unsigned int grabmask1 = ControlMask|Mod1Mask;
unsigned int grabmask2 = Mod1Mask;
unsigned int altmask = ShiftMask;

/* Client tracking information */
Client          *head_client = NULL;
Client          *current = NULL;
volatile Window initialising = None;

static void setup_display(void);
static void *xmalloc(size_t size);
static void parse_rcfile(FILE *fp);

char *xstrcpy(const char *str);
char *xstrcpy(const char *str)
{
    char *tmp;

    if (!str) return NULL;

    tmp = (char *)malloc(strlen(str));
    return strcpy(tmp, str);
}

void parse_rcfile(FILE *fp) {
#define RC_LINE_LEN 256
  char *line = malloc(RC_LINE_LEN);
  int len;

  if (!line) return;

  while (1)
  {
    (void) fgets(line, RC_LINE_LEN, fp);
    if (feof(fp)) break;
    len = strlen(line);
    if (len > 0) {
      if (line[len-1] == '\n') line[len-1] = '\0';
      command_execute( line );
    }
  }
  free(line);
#undef RC_LINE_LEN
}

int main(int argc, char *argv[]) {
	struct sigaction act;

  settings_init();

  if ( !commandline_process( argc, argv ) ) exit( -1 );

  command_init();
  evilpoison_commands_init();

	char *homedir = getenv("HOME");

	if (homedir) {
	    char *rcfile = (char *)malloc(strlen(homedir)+strlen("/.evilpoisonrc")+2);
	    FILE *rcfp = NULL;

	    if (rcfile) {
		sprintf(rcfile, "%s/.evilpoisonrc", homedir);

		if ((rcfp = fopen(rcfile, "r"))) {
		    parse_rcfile(rcfp);
		    fclose(rcfp);
		}
	    }
	    free(rcfile);
	}

	act.sa_handler = handle_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGHUP, &act, NULL);

	setup_display();

  command_execute( "echo Welcome to EvilPoison $version$" );

	event_main_loop();

	return 1;
}

static void *xmalloc(size_t size) {
	void *ptr = malloc(size);
	if (!ptr) {
		/* C99 defines the 'z' printf modifier for variables of
		 * type size_t.  Fall back to casting to unsigned long. */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
		LOG_ERROR("out of memory, looking for %zu bytes\n", size);
#else
		LOG_ERROR("out of memory, looking for %lu bytes\n",
				(unsigned long)size);
#endif
		exit(1);
	}
	return ptr;
}

static void setup_display(void) {
	XSetWindowAttributes attr;
	XModifierKeymap *modmap;
	/* used in scanning windows (XQueryTree) */
	unsigned int i, j, nwins;
	Window dw1, dw2, *wins;
	XWindowAttributes winattr;

	dpy = XOpenDisplay( settings_get( "display") );
	if (!dpy) { 
		LOG_ERROR("can't open display %s\n", settings_get( "display") );
		exit(1);
	}
	XSetErrorHandler(handle_xerror);
	/* XSynchronize(dpy, True); */

	/* Standard X protocol atoms */
	xa_wm_state = XInternAtom(dpy, "WM_STATE", False);
	xa_wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);
	xa_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	xa_wm_cmapwins = XInternAtom(dpy, "WM_COLORMAP_WINDOWS", False);
	/* Motif atoms */
	mwm_hints = XInternAtom(dpy, _XA_MWM_HINTS, False);
	/* EWMH atoms */
	xa_net_wm_desktop = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	xa_net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	xa_net_wm_state_sticky = XInternAtom(dpy, "_NET_WM_STATE_STICKY", False);

  XFontStruct *font;
	font = XLoadQueryFont(dpy, settings_get( "text.font" ) );
	if (!font)
		LOG_ERROR("[Warning] Couldn't find a font to use try starting with a different font.\n")
  else
    XFreeFont( dpy, font );

	move_curs = XCreateFontCursor(dpy, XC_fleur);
	resize_curs = XCreateFontCursor(dpy, XC_plus);

	/* find out which modifier is NumLock - we'll use this when grabbing
	 * every combination of modifiers we can think of */
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < (unsigned int)modmap->max_keypermod; j++) {
			if (modmap->modifiermap[i*modmap->max_keypermod+j] == XKeysymToKeycode(dpy, XK_Num_Lock)) {
				numlockmask = (1<<i);
				LOG_DEBUG("setup_display() : XK_Num_Lock is (1<<0x%02x)\n", i);
			}
		}
	}
	XFreeModifiermap(modmap);

	/* set up root window attributes - same for each screen */
	attr.event_mask = ChildMask | ColormapChangeMask;
	if (atoi(settings_get("mouse.focus")))
	    attr.event_mask |= EnterWindowMask;

	/* SHAPE extension? */
	{
		int e_dummy;
		have_shape = XShapeQueryExtension(dpy, &shape_event, &e_dummy);
	}

	/* now set up each screen in turn */
	num_screens = ScreenCount(dpy);
	if (num_screens < 0) {
		LOG_ERROR("Can't count screens\n");
		exit(1);
	}
	screens = xmalloc(num_screens * sizeof(ScreenInfo));
	for (i = 0; i < (unsigned int)num_screens; i++) {
		char *ds, *colon, *dot;
		ds = DisplayString(dpy);
		/* set up DISPLAY environment variable to use */
		colon = strrchr(ds, ':');
		if (colon && num_screens > 1) {
			screens[i].display = xmalloc(14 + strlen(ds));
			strcpy(screens[i].display, "DISPLAY=");
			strcat(screens[i].display, ds);
			colon = strrchr(screens[i].display, ':');
			dot = strchr(colon, '.');
			if (!dot)
				dot = colon + strlen(colon);
			snprintf(dot, 5, ".%d", i);
		} else
			screens[i].display = NULL;

		screens[i].screen = i;
		screens[i].root = RootWindow(dpy, i);
		screens[i].vdesk = 0;
		screens[i].other_vdesk = 0;

		XChangeWindowAttributes(dpy, screens[i].root, CWEventMask, &attr);
		grab_keys_for_screen(&screens[i]);

		/* scan all the windows on this screen */
		LOG_XDEBUG("main:XQueryTree(); ");
		XQueryTree(dpy, screens[i].root, &dw1, &dw2, &wins, &nwins);
		LOG_XDEBUG("%d windows\n", nwins);
		for (j = 0; j < nwins; j++) {
			XGetWindowAttributes(dpy, wins[j], &winattr);
			if (!winattr.override_redirect && winattr.map_state == IsViewable)
				make_new_client(wins[j], &screens[i]);
		}
		XFree(wins);
	}
}
