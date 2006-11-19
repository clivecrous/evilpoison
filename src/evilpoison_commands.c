/** EvilPoison Commands implementation.
 * \file evilpoison_commands.c
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "command.h"
#include "settings.h"
#include "evilpoison_commands.h"
#include "bind.h"
#include "evilpoison.h"

char *evilpoison_command_set(char *commandline)
{
  char *newcommandline = malloc( strlen( commandline ) + 1 );
  strcpy( newcommandline, commandline );

  char *setting = newcommandline;
  while (*setting && *setting==' ' ) setting++;

  char *value = setting;
  while (*value && !isblank(*value) ) value++;
  while (*value && isblank(*value) ) { *value='\0'; value++; };

  settings_set( setting, value );

  free( newcommandline );

  return 0;
}

char *evilpoison_command_unset(char *commandline)
{
  settings_unset( commandline );

  return 0;
}

char *evilpoison_command_bind(char *commandline)
{
  BindKeySymMask *binding;
  char *params = malloc( strlen( commandline ) + 1 );
  strcpy( params, commandline );

  char *tmpc = params;
  while (*tmpc && !isblank(*tmpc)) tmpc++;
  while (*tmpc && isblank(*tmpc)) { *tmpc = '\0'; tmpc++; }

  binding = keycode_convert( params );
  if ( binding )
  {
    add_key_binding( binding->symbol, binding->mask, tmpc);
    free( binding );
  }

  free( params );
  return 0;
}

char *evilpoison_command_cmdmode(char *commandline)
{
  global_cmdmode = !global_cmdmode;
  return 0;
}

char *evilpoison_command_exec(char *commandline)
{
  spawn( commandline );
  return 0;
}

char *evilpoison_command_info(char *commandline)
{
  if (current) show_info(current);
  return 0;
}

char *evilpoison_command_nextwin(char *commandline)
{
  next();
  return 0;
}

char *evilpoison_command_max(char *commandline)
{
    if (current) maximise_client(current, MAXIMISE_HORZ|MAXIMISE_VERT);
    return 0;
}

char *evilpoison_command_maxvert(char *commandline)
{
    if (current) maximise_client(current, MAXIMISE_VERT);
    return 0;
}

char *evilpoison_command_maxhoriz(char *commandline)
{
    if (current) maximise_client(current, MAXIMISE_HORZ);
    return 0;
}

#ifdef VWM
char *evilpoison_command_fix(char *commandline)
{
    if (current) fix_client(current);
    return 0;
}

char *evilpoison_command_prevdesk(char *commandline)
{
    ScreenInfo *current_screen = find_current_screen();
    if (current_screen->vdesk > 0 )
	switch_vdesk(current_screen, current_screen->vdesk - 1);
    return 0;
}

char *evilpoison_command_nextdesk(char *commandline)
{
    ScreenInfo *current_screen = find_current_screen();
    if (current_screen->vdesk < 7 )
	switch_vdesk(current_screen, current_screen->vdesk + 1);
    return 0;
}
#endif

static void apply_client_position(Client *client) {
    moveresize(client);
    if ( atoi( settings_get( "mouse.warp" ) ) )
      setmouse( client->window,
          client->width + client->border - 1,
          client->height + client->border - 1);
    discard_enter_events();
}

char *evilpoison_command_window_move(char *commandline)
{
  if (!current) return 0;

  char *x_str = command_parameter_copy( commandline, 0 );
  char *y_str = command_parameter_copy( commandline, 1 );

  if ( !x_str ) {
    if ( y_str ) free( y_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }
  if ( !y_str ) {
    free( x_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }

  current->x += atoi( x_str );
  current->y += atoi( y_str );
  apply_client_position( current );

  free( x_str );
  free( y_str );

  return 0;
}

char *evilpoison_command_window_moveto(char *commandline)
{
  if (!current) return 0;

  char *x_str = command_parameter_copy( commandline, 0 );
  char *y_str = command_parameter_copy( commandline, 1 );
  int x,y;

  if ( !x_str ) {
    if ( y_str ) free( y_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }
  if ( !y_str ) {
    free( x_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }

  if ( *x_str == 'X' )
    x = DisplayWidth(dpy, current->screen->screen) -
          ( current->width + current->border*2 ) / 2;
  else
    x = atoi( x_str );

  if ( *y_str == 'X' )
    y = DisplayHeight(dpy, current->screen->screen) -
          ( current->height + current->border*2 ) / 2;
  else
    y = atoi( y_str );

  /* It may seem strange that I check for the string negative instead of <0 for
   * the integer value below. The "logic" of it is quite simple really: an
   * atoi("-0") returns as 0. I want -0 to mean against-the-right or
   * on-the-bottom so I need to check if there was a negative in the string.
   */
  if ( *x_str=='-' )
    x += DisplayWidth(dpy, current->screen->screen) -
      ( current->width + current->border );
  if ( *y_str=='-' )
    y += DisplayHeight(dpy, current->screen->screen) -
      ( current->height + current->border );

  current->x = x;
  current->y = y;
  apply_client_position( current );

  free( x_str );
  free( y_str );

  return 0;
}

char *evilpoison_command_window_resize(char *commandline)
{
  if (!current) return 0;

  char *top_str = command_parameter_copy( commandline, 0 );
  if ( !top_str )
  {
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }

  char *left_str = command_parameter_copy( commandline, 1 );
  if ( !left_str )
  {
    free( top_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }

  char *right_str = command_parameter_copy( commandline, 2 );
  if ( !right_str )
  {
    free( top_str );
    free( left_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }
  char *bottom_str = command_parameter_copy( commandline, 3 );
  if ( !bottom_str )
  {
    free( top_str );
    free( left_str );
    free( right_str );
    /** \todo Should probably display an error once echo is implemented? */
    return 0;
  }

  current->y += atoi( top_str );
  current->x += atoi( left_str );
  current->width += atoi( right_str );
  current->height += atoi( bottom_str );

  apply_client_position( current );

  free( top_str );
  free( left_str );
  free( right_str );
  free( bottom_str );
}

char *evilpoison_command_window_move_mouse(char *commandline)
{
  if ( current ) drag( current );
  return 0;
}

char *evilpoison_command_window_resize_mouse(char *commandline)
{
  if ( current ) sweep( current );
  return 0;
}

char *evilpoison_command_window_close(char *commandline)
{
  if ( current ) send_wm_delete(current, 0);
  return 0;
}

char *evilpoison_command_window_kill(char *commandline)
{
  if ( current ) send_wm_delete(current, 1);
  return 0;
}

char *evilpoison_command_desk_switch(char *commandline)
{
  ScreenInfo *current_screen = find_current_screen();

  char *desk_str = command_parameter_copy( commandline, 0 );
  if (!desk_str) return 0;

  switch_vdesk(current_screen, atoi( desk_str ) );
  return 0;
}

void evilpoison_commands_init( void )
{
  command_assign( "set",    evilpoison_command_set );
  command_assign( "unset",  evilpoison_command_unset );

  command_assign( "bind",   evilpoison_command_bind );
  command_assign( "cmdmode",   evilpoison_command_cmdmode );
  command_assign( "exec",   evilpoison_command_exec );
  command_assign( "info",   evilpoison_command_info );

  command_assign( "desk.switch",   evilpoison_command_desk_switch );

  command_assign( "nextwin",   evilpoison_command_nextwin );

  command_assign( "max",    evilpoison_command_max );
  command_assign( "maxvert",    evilpoison_command_maxvert );
  command_assign( "maxhoriz",    evilpoison_command_maxhoriz );

#ifdef VWM
  command_assign( "fix",    evilpoison_command_fix );
  command_assign( "nextdesk",    evilpoison_command_nextdesk );
  command_assign( "prevdesk",    evilpoison_command_prevdesk );
#endif

  command_assign( "window.close", evilpoison_command_window_close );
  command_assign( "window.kill", evilpoison_command_window_kill );

  command_assign( "window.move", evilpoison_command_window_move );
  command_assign( "window.moveto", evilpoison_command_window_moveto );

  command_assign( "window.resize", evilpoison_command_window_resize );

  command_assign( "window.move.mouse",evilpoison_command_window_move_mouse );
  command_assign( "window.resize.mouse",
      evilpoison_command_window_resize_mouse );
}

