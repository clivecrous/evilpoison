/* evilwm - Minimalist Window Manager for X
 * Copyright (C) 1999-2006 Ciaran Anscomb <evilwm@6809.org.uk>
 * see README for license and other details. */

#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bind.h"
#include "evilpoison.h"
#include "log.h"

int need_client_tidy = 0;
int ignore_xerror = 0;

/* Now do this by fork()ing twice so we don't have to worry about SIGCHLDs */
void spawn(const char * command) {
  ScreenInfo *current_screen = find_current_screen();
  pid_t pid;

  if (current_screen && current_screen->display)
    putenv(current_screen->display);

  if (!(pid = fork())) {
    setsid();
    if (!fork()) execlp( "sh", "sh", "-c", command, NULL );
    else _exit(0);
  }

  if (pid > 0)
    wait(NULL);
}

int handling_signal = 0; //TODO mutex this

void handle_signal(int signo) {
  int i;
  (void)signo;  /* unused */

  if ( handling_signal != 0 ) return;
  handling_signal = 1;

  /* SIGCHLD check no longer necessary */
  /* Quit Nicely */
  while(head_client) remove_client(head_client);
  XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
  for (i = 0; i < num_screens; i++)
    XInstallColormap(dpy, DefaultColormap(dpy, i));
  free(screens);
  printf("Good Bye.\n");
  exit(0);
}

int handle_xerror(Display *dsply, XErrorEvent *e) {
  Client *c;
  (void)dsply;  /* unused */

  char errorText[1024];

  XGetErrorText( dpy, e->error_code, errorText, sizeof(errorText) );

  if ( e->error_code == 3 )
  {
    LOG_DEBUG("handle_xerror() - ignoring BadWindow error, this is probably just a window that was recently closed\n");
    return 0;
  }

  if (ignore_xerror) {
    LOG_DEBUG("handle_xerror() ignored an XErrorEvent: %d\n", e->error_code);
    return 0;
  }

  /* If this error actually occurred while setting up the new
   * window, best let make_new_client() know not to bother */
  if (initialising != None && e->resourceid == initialising) {
    LOG_DEBUG("\t **SAVED?** handle_xerror() caught error %d while initialising\n", e->error_code);
    initialising = None;
    return 0;
  }

  LOG_DEBUG("**********************************************************\n");
  LOG_DEBUG("X ERROR:\n");
  LOG_DEBUG("\t%s\n",errorText);
  LOG_DEBUG("\ttype: %d\n",e->type);
  LOG_DEBUG("\terror code: %d\n",e->error_code);
  LOG_DEBUG("\trequest code: %d.%d\n",e->request_code, e->minor_code);
  LOG_DEBUG("**********************************************************\n");

  if (e->error_code == BadAccess && e->request_code == X_ChangeWindowAttributes) {
    LOG_ERROR("root window unavailable (maybe another wm is running?)\n");
    exit(1);
  }

  c = find_client(e->resourceid);
  if (c) {
    LOG_DEBUG("\thandle_xerror() : flagging client for removal\n");
    c->remove = 1;
    need_client_tidy = 1;
  }
  return 0;
}
