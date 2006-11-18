/** Command implementation.
 * \file command.c
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "command.h"

Dictionary *_commands = 0;

static void commands_done( void );

void command_init( void )
{
  if (_commands) return;
  _commands = dictionary_create();
  atexit( commands_done );
}

static void commands_done( void )
{
  dictionary_destroy( _commands );
}

void command_assign(const char *command, CommandFunction function)
{
  CommandFunction *func = malloc( sizeof( CommandFunction ) );
  *func = function;
  dictionary_set( _commands, command, func );
}

void command_unassign(const char *command)
{
  dictionary_unset( _commands, command );
}

char *command_execute(const char *commandline)
{
  char *return_value;
  CommandFunction *function;
  if (!commandline || !strlen(commandline)) return 0;

  char *newcommandline = malloc( strlen( commandline ) + 1 );
  strcpy( newcommandline, commandline );

  char *command = newcommandline;
  while (*command && isblank(*command)) command++;

  char *parameters = command;
  while (*parameters && !isblank(*parameters)) parameters++;
  while (*parameters && isblank(*parameters))
  { *parameters='\0'; parameters++; };

  function = dictionary_get( _commands, command );
  if (function)
    return_value = (*function)( parameters );
  else
    return_value = 0;

  free( newcommandline );

  return return_value;
}
