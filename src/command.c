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

char *command_parameter_copy(
    const char *parameters, const unsigned int which_parameter)
{
  const char *parameter_start_position;
  const char *parameter_end_position;
  unsigned int parameter_enum;
  char *copyof_parameter;

  if (!parameters) return 0;

  for ( parameter_start_position=parameters;
      *parameter_start_position && isblank(*parameter_start_position); parameter_start_position++);
  if (!*parameter_start_position) return 0;

  for ( parameter_enum=0; parameter_enum < which_parameter; parameter_enum++)
  {
    for (; *parameter_start_position && !isblank(*parameter_start_position); parameter_start_position++);
    for (; *parameter_start_position && isblank(*parameter_start_position); parameter_start_position++);
  }
  if (!*parameter_start_position) return 0;

  for (parameter_end_position=parameter_start_position;
      *parameter_end_position && !isblank(*parameter_end_position);
      parameter_end_position++);

  if (parameter_end_position-parameter_start_position == 0) return 0;

  copyof_parameter = malloc(parameter_end_position-parameter_start_position+1);
  strncpy( copyof_parameter, parameter_start_position,
      parameter_end_position-parameter_start_position );
  copyof_parameter[parameter_end_position-parameter_start_position] = '\0';

  return copyof_parameter;
}
