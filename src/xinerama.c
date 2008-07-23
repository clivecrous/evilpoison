#include <stdlib.h>
#include <stdio.h>

#include <X11/extensions/Xinerama.h>

#include "evilpoison.h"
#include "xinerama.h"

static XineramaScreenInfo * xinerama_screens = NULL;
static int xinerama_screen_amount = 0;

static void xinerama_done( void );
static XineramaScreenInfo * xinerama_current_screen( void );

void xinerama_init( void )
{
  atexit( xinerama_done );
  if ( !XineramaIsActive( dpy ) ) return;
  xinerama_screens = XineramaQueryScreens( dpy, &xinerama_screen_amount );
}

int xinerama_screen_origin_x( void )
{
  XineramaScreenInfo * current_screen = xinerama_current_screen();
  if ( !current_screen ) return 0;
  return current_screen->x_org;
}

int xinerama_screen_origin_y( void )
{
  XineramaScreenInfo * current_screen = xinerama_current_screen();
  if ( !current_screen ) return 0;
  return current_screen->y_org;
}

int xinerama_screen_width( void )
{
  XineramaScreenInfo * current_xineramas_screen= xinerama_current_screen();
  if ( !current_xineramas_screen )
  {
    ScreenInfo *current_screen = find_current_screen();
    return DisplayWidth( dpy, current_screen->screen );
  }
  else
  {
    return current_xineramas_screen->width;
  }
} 

int xinerama_screen_height( void )
{
  XineramaScreenInfo * current_xineramas_screen= xinerama_current_screen();
  if ( !current_xineramas_screen )
  {
    ScreenInfo *current_screen = find_current_screen();
    return DisplayHeight( dpy, current_screen->screen );
  }
  else
  {
    return current_xineramas_screen->height;
  }
} 

static XineramaScreenInfo * xinerama_current_screen( void )
{
  if ( ! xinerama_screens ) return NULL;

	Window dummy_window;
  int dummy_int;
	unsigned int dummy_unsigned_int;

	int pointer_x_coordinate, pointer_y_coordinate;
  int head_counter;

	XQueryPointer(
      dpy, DefaultRootWindow( dpy ),
      &dummy_window, &dummy_window,
      &pointer_x_coordinate, &pointer_y_coordinate,
      &dummy_int, &dummy_int, &dummy_unsigned_int);

  XineramaScreenInfo * xinerama_screen_iterator = xinerama_screens;

  for ( head_counter = 0; head_counter < xinerama_screen_amount; head_counter++ )
  {
    if ( pointer_x_coordinate >= xinerama_screen_iterator->x_org &&
         pointer_y_coordinate >= xinerama_screen_iterator->y_org &&
         pointer_x_coordinate <
           xinerama_screen_iterator->x_org + xinerama_screen_iterator->width &&
         pointer_y_coordinate <
           xinerama_screen_iterator->y_org + xinerama_screen_iterator->height
       ) return xinerama_screen_iterator;
    xinerama_screen_iterator++;
  }

  return NULL;
}

static void xinerama_done( void )
{
  if ( xinerama_screens ) XFree( xinerama_screens );
}
