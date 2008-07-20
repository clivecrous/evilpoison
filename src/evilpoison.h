#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>

/* Required for interpreting MWM hints: */
#define _XA_MWM_HINTS           "_MOTIF_WM_HINTS"
#define PROP_MWM_HINTS_ELEMENTS 3
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
typedef struct {
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
} PropMwmHints;

/* sanity on options */
#define INFOBANNER

/* default settings */

#define SPACE           3

#define DEF_PREFIX_KEY  XK_e
#define DEF_PREFIX_MOD  ControlMask;

/* readability stuff */
#define RAISE           1
#define NO_RAISE        0       /* for unhide() */

#define MAXIMISE_HORZ        (1<<0)
#define MAXIMISE_VERT        (1<<1)

/* some coding shorthand */

#define ChildMask       (SubstructureRedirectMask|SubstructureNotifyMask)
#define ButtonMask      (ButtonPressMask|ButtonReleaseMask)
#define MouseMask       (ButtonMask|PointerMotionMask)

#define grab_pointer(w, mask, curs) \
	(XGrabPointer(dpy, w, False, mask, GrabModeAsync, GrabModeAsync, \
	None, curs, CurrentTime) == GrabSuccess)
#define grab_button(w, mask, button) do { \
		XGrabButton(dpy, button, (mask), w, False, ButtonMask, \
			    GrabModeAsync, GrabModeSync, None, None); \
		XGrabButton(dpy, button, LockMask|(mask), w, False, ButtonMask,\
			    GrabModeAsync, GrabModeSync, None, None); \
		XGrabButton(dpy, button, numlockmask|(mask), w, False, \
			    ButtonMask, GrabModeAsync, GrabModeSync, \
			    None, None); \
		XGrabButton(dpy, button, numlockmask|LockMask|(mask), w, False,\
			    ButtonMask, GrabModeAsync, GrabModeSync, \
			    None, None); \
	} while (0)
#define setmouse(w, x, y) XWarpPointer(dpy, None, w, 0, 0, 0, 0, x, y)
#define get_mouse_position(xp,yp,root) do { \
		Window dw; \
		int di; \
		unsigned int dui; \
		XQueryPointer(dpy, root, &dw, &dw, xp, yp, &di, &di, &dui); \
	} while (0)
#define gravitate(c) gravitate_client(c, 1)
#define ungravitate(c) gravitate_client(c, -1)

#define is_sticky(c) (c->sticky)
#define add_sticky(c) c->sticky = 1
#define remove_sticky(c) c->sticky = 0
#define toggle_sticky(c) c->sticky = !c->sticky

#define discard_enter_events() do { \
		XEvent dummy; \
		XSync(dpy, False); \
		while (XCheckMaskEvent(dpy, EnterWindowMask, &dummy)); \
	} while (0)

/* screen structure */

typedef struct ScreenInfo ScreenInfo;
struct ScreenInfo {
	int screen;
	Window root;
	int vdesk;
	int other_vdesk;
	char *display;
};

/* client structure */

typedef struct Client Client;
struct Client {
	Window  window;
	Window  parent;
	ScreenInfo      *screen;
	Colormap        cmap;
	int             ignore_unmap;

	int             x, y, width, height;
	int             border;
	int             oldx, oldy, oldw, oldh;  /* used when maximising */

	int             min_width, min_height;
	int             max_width, max_height;
	int             width_inc, height_inc;
	int             base_width, base_height;
	int             win_gravity;
	int             old_border;
	int             vdesk;
	int             sticky;
	int             remove;  /* set when client needs to be removed */
	Client  *next;
};

/* Declarations for global variables in main.c */

/* Commonly used X information */
extern Display      *dpy;
extern Cursor       move_curs;
extern Cursor       resize_curs;
extern int          num_screens;
extern ScreenInfo   *screens;
extern int          have_shape, shape_event;

/* Standard X protocol atoms */
extern Atom xa_wm_state;
extern Atom xa_wm_protos;
extern Atom xa_wm_delete;
extern Atom xa_wm_cmapwins;
/* Motif atoms */
extern Atom mwm_hints;
/* EWMH atoms */
extern Atom xa_net_wm_desktop;
extern Atom xa_net_wm_state;
extern Atom xa_net_wm_state_sticky;

/* Things that affect user interaction */
extern unsigned int     numlockmask;
extern unsigned int     grabmask1;
extern unsigned int     grabmask2;
extern unsigned int     altmask;

/* Client tracking information */
extern Client           *head_client;
extern Client           *current;
extern volatile Window  initialising;

/* client.c */

Client *find_client(Window w);
void gravitate_client(Client *c, int sign);
void select_client(Client *c);
void float_client(Client *c);
void remove_client(Client *c);
void send_config(Client *c);
void send_wm_delete(Client *c, int kill_client);
void set_wm_state(Client *c, int state);
void set_shape(Client *c);

/* events.c */

extern int global_cmdmode;
void event_main_loop(void);
void add_key_binding(KeySym k, unsigned int mask, char *cmd);
void free_key_bindings(void);
int find_key_binding(KeySym k, unsigned int mask);

/* misc.c */

extern int need_client_tidy;
extern int ignore_xerror;
int handle_xerror(Display *dsply, XErrorEvent *e);
void spawn(const char * command);
void handle_signal(int signo);

/* new.c */

void make_new_client(Window w, ScreenInfo *s);
long get_wm_normal_hints(Client *c);

/* screen.c */

void drag(Client *c);
void moveresize(Client *c);
void maximise_client(Client *c, int hv);
void show_info(Client *c);
void sweep(Client *c);
void unhide(Client *c, int raise_win);
void next(void);
void previous(void);
void hide(Client *c);
void switch_vdesk(ScreenInfo *s, int v);
ScreenInfo *find_screen(Window root);
ScreenInfo *find_current_screen(void);
void grab_keys_for_screen(ScreenInfo *s);

/* ewmh.c */

void update_net_wm_desktop(Client *c);
void update_net_wm_state(Client *c);
