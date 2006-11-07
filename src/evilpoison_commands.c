#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "settings.h"
#include "evilpoison_commands.h"

char *evilpoison_command_set(char *commandline);
char *evilpoison_command_unset(char *commandline);

char *evilpoison_command_set(char *commandline)
{
  char *newcommandline = malloc( strlen( commandline ) + 1 );
  strcpy( newcommandline, commandline );

  char *setting = newcommandline;
  while (*setting && *setting==' ' ) setting++;

  char *value = setting;
  while (*value && *value!=' ' ) value++;
  while (*value && *value==' ' ) { *value='\0'; value++; };

  settings_set( setting, value );

  free( newcommandline );

  return 0;
}

char *evilpoison_command_unset(char *commandline)
{
  settings_unset( commandline );

  return 0;
}

void evilpoison_commands_init( void )
{
  command_assign( "set",    evilpoison_command_set );
  command_assign( "unset",  evilpoison_command_unset );
}
