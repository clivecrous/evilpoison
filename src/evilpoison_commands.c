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

void evilpoison_commands_init( void )
{
  command_assign( "set",    evilpoison_command_set );
  command_assign( "unset",  evilpoison_command_unset );

  command_assign( "bind",   evilpoison_command_bind );
  command_assign( "cmdmode",   evilpoison_command_cmdmode );
  command_assign( "exec",   evilpoison_command_exec );
  command_assign( "info",   evilpoison_command_info );

  command_assign( "nextwin",   evilpoison_command_nextwin );

  command_assign( "max",    evilpoison_command_max );
  command_assign( "maxvert",    evilpoison_command_maxvert );
  command_assign( "maxhoriz",    evilpoison_command_maxhoriz );

#ifdef VWM
  command_assign( "fix",    evilpoison_command_fix );
  command_assign( "nextdesk",    evilpoison_command_nextdesk );
  command_assign( "prevdesk",    evilpoison_command_prevdesk );
#endif
}
