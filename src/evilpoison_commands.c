/** EvilPoison Commands implementation.
 * \file evilpoison_commands.c
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

#include "command.h"
#include "settings.h"
#include "evilpoison_commands.h"
#include "bind.h"
#include "evilpoison.h"
#include "xinerama.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#else
# define UNUSED(x) x
#endif

char *evilpoison_command_echo(char *commandline)
{
  internal_echo( commandline );
  return 0;
}

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

char *evilpoison_command_bind_keyboard(char *commandline)
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

char *evilpoison_command_mode_command(char *UNUSED(commandline))
{
  global_mode = mode_command;
  return 0;
}

char *evilpoison_command_execute_fork(char *commandline)
{
  spawn( commandline );
  return 0;
}

char *evilpoison_command_execute_here(char *commandline)
{
#define MAX_LINE_LENGTH 1024
  FILE *execution = popen( commandline, "r" );
  if (!execution) return 0;

  char *result = NULL;
  char *line = malloc( MAX_LINE_LENGTH );

  while (1)
  {
    fgets( line, MAX_LINE_LENGTH, execution );
    if ( feof( execution ) ) break;

    /* strip newline characters from the end of the line */
    while ( strlen( line ) > 0 && ( line[strlen(line)-1] == 0x0D || line[strlen(line)-1] == 0x0A ) )
      line[strlen(line)-1] = 0x00;

    if ( strlen( line ) == 0 ) continue;

    if ( result )
      result = realloc( result, strlen( result ) + strlen( line ) + 1 );
    else
    {
      result = malloc( strlen( line ) + 1 );
      result[0]='\0';
    }
    strcat( result, line );
  }

  pclose( execution );
  free( line );

  return result;
#undef MAX_LINE_LENGTH
}

char *evilpoison_command_window_info(char *UNUSED(commandline))
{
  if (current) show_info(current);
  return 0;
}

char *evilpoison_command_window_next(char *UNUSED(commandline))
{
  next();
  return 0;
}

char *evilpoison_command_window_previous(char *UNUSED(commandline))
{
  previous();
  return 0;
}

char *evilpoison_command_window_maximize(char *UNUSED(commandline))
{
    if (current) maximise_client(current, MAXIMISE_HORZ|MAXIMISE_VERT);
    return 0;
}

char *evilpoison_command_window_maximize_vertical(char *UNUSED(commandline))
{
    if (current) maximise_client(current, MAXIMISE_VERT);
    return 0;
}

char *evilpoison_command_window_maximize_horizontal(char *UNUSED(commandline))
{
    if (current) maximise_client(current, MAXIMISE_HORZ);
    return 0;
}

char *evilpoison_command_window_float(char *UNUSED(commandline))
{
    if (current) float_client(current);
    return 0;
}

char *evilpoison_command_desktop_navigate_previous(char *UNUSED(commandline))
{
    ScreenInfo *current_screen = find_current_screen();
    if (current_screen->virtual_desktop > 0 )
	switch_virtual_desktop(current_screen, current_screen->virtual_desktop - 1);
    return 0;
}

char *evilpoison_command_desktop_navigate_next(char *UNUSED(commandline))
{
    ScreenInfo *current_screen = find_current_screen();
    if (current_screen->virtual_desktop < 7 )
	switch_virtual_desktop(current_screen, current_screen->virtual_desktop + 1);
    return 0;
}

char *evilpoison_command_desktop_history_back(char *UNUSED(commandline))
{
    ScreenInfo *current_screen = find_current_screen();
    if (current_screen->virtual_desktop != current_screen->other_virtual_desktop)
	switch_virtual_desktop(current_screen, current_screen->other_virtual_desktop);
    return 0;
}

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

  int screen_origin_x = xinerama_screen_origin_x();
  int screen_origin_y = xinerama_screen_origin_y();

  int screen_width = xinerama_screen_width();
  int screen_height = xinerama_screen_height();

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
    x = ( screen_width - ( current->width + current->border*2 ) ) / 2;
  else
    x = atoi( x_str );

  if ( *y_str == 'X' )
    y = ( screen_height - ( current->height + current->border*2 ) ) / 2;
  else
    y = atoi( y_str );

  /* It may seem strange that I check for the string negative instead of <0 for
   * the integer value below. The "logic" of it is quite simple really: an
   * atoi("-0") returns as 0. I want -0 to mean against-the-right or
   * on-the-bottom so I need to check if there was a negative in the string.
   */
  if ( *x_str=='-' )
    x += screen_width - (current->width + current->border);
  else
    x += current->border;
  if ( *y_str=='-' )
    y += screen_height - (current->height + current->border );
  else
    y += current->border;

  current->x = screen_origin_x + x;
  current->y = screen_origin_y + y;

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

  return 0;
}

char *evilpoison_command_window_move_mouse(char *UNUSED(commandline))
{
  if ( current ) drag( current );
  return 0;
}

char *evilpoison_command_window_resize_mouse(char *UNUSED(commandline))
{
  if ( current ) sweep( current );
  return 0;
}

char *evilpoison_command_window_close(char *UNUSED(commandline))
{
  if ( current ) send_wm_delete(current, 0);
  return 0;
}

char *evilpoison_command_window_kill(char *UNUSED(commandline))
{
  if ( current ) send_wm_delete(current, 1);
  return 0;
}

char *evilpoison_command_window_lower(char *UNUSED(commandline))
{
  if ( current ) XLowerWindow(dpy, current->parent);
  return 0;
}

char *evilpoison_command_desktop_navigate_to(char *commandline)
{
  ScreenInfo *current_screen = find_current_screen();

  char *desktop_str = command_parameter_copy( commandline, 0 );
  if (!desktop_str) return 0;

  int desktop = atoi( desktop_str ) - 1;
  if ( desktop >= 0 && desktop <= 7 ) switch_virtual_desktop( current_screen, desktop );

  return 0;
}

char *evilpoison_command_alias(char *commandline)
{
  char *command;
  for ( command=commandline; *command && isblank(*command); command++ );
  for ( ;*command && !isblank(*command); command++ );
  for ( ;*command && isblank(*command); command++ );

  if (!*command) return 0;

  char *alias = command_parameter_copy( commandline, 0 );
  alias_assign( alias, command );
  free( alias );

  return 0;
}

void evilpoison_commands_init( void )
{
  command_assign( "alias",    evilpoison_command_alias );

  command_assign( "set",    evilpoison_command_set );
  command_assign( "unset",  evilpoison_command_unset );

  command_assign( "bind.keyboard",   evilpoison_command_bind_keyboard );
  command_assign( "mode.command",   evilpoison_command_mode_command );
  command_assign( "echo",   evilpoison_command_echo );
  command_assign( "execute.fork",   evilpoison_command_execute_fork );
  command_assign( "execute.here",   evilpoison_command_execute_here );
  command_assign( "window.info",   evilpoison_command_window_info );

  command_assign( "window.next",   evilpoison_command_window_next );
  command_assign( "window.previous",   evilpoison_command_window_previous );

  command_assign( "window.maximize",
      evilpoison_command_window_maximize );
  command_assign( "window.maximize.vertical",
      evilpoison_command_window_maximize_vertical );
  command_assign( "window.maximize.horizontal",
      evilpoison_command_window_maximize_horizontal );

  command_assign( "window.float",    evilpoison_command_window_float );

  command_assign( "desktop.navigate.next",    evilpoison_command_desktop_navigate_next );
  command_assign( "desktop.navigate.previous",    evilpoison_command_desktop_navigate_previous );
  command_assign( "desktop.navigate.to",   evilpoison_command_desktop_navigate_to );
  command_assign( "desktop.history.back",   evilpoison_command_desktop_history_back );

  command_assign( "window.close", evilpoison_command_window_close );
  command_assign( "window.kill", evilpoison_command_window_kill );

  command_assign( "window.lower", evilpoison_command_window_lower );

  command_assign( "window.move", evilpoison_command_window_move );
  command_assign( "window.moveto", evilpoison_command_window_moveto );

  command_assign( "window.resize", evilpoison_command_window_resize );

  command_assign( "window.move.mouse",evilpoison_command_window_move_mouse );
  command_assign( "window.resize.mouse",
      evilpoison_command_window_resize_mouse );
}

