/** Command implementation.
 * \file command.c
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "command.h"
#include "settings.h"

Dictionary *_aliases = 0;
Dictionary *_commands = 0;

static void commands_done( void );

void command_init( void )
{
  if (_commands) return;
  _aliases = dictionary_create();
  _commands = dictionary_create();
  atexit( commands_done );
}

static void commands_done( void )
{
  dictionary_destroy( _aliases );
  dictionary_destroy( _commands );
}

void alias_assign(const char *alias, const char *commandline)
{
  char *cmdline = malloc( strlen( commandline ) + 1 );
  strcpy( cmdline, commandline );
  cmdline[ strlen( commandline ) ] = '\0';
  dictionary_set( _aliases, alias, cmdline );
}

void alias_unassign(const char *alias)
{
  dictionary_unset( _aliases, alias );
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

/** Parse commandline for variables.
 * If a variable named 'foo' exists with the value of "bar" then the following
 * commandline, as given to this parser:
 *    hello $foo$ world
 * will result in the following:
 *    hello bar world
 * a double $$ will result in a single $   
 * \param commandline Any normal commandline.
 * \return A commandline with variable values injected.
 */
static char *command_parse_commandline( const char *commandline )
{
  const char *seek_start;
  const char *seek_end;

  char *result;
  char *result_position;
  unsigned int result_size;

  if (!commandline) return 0;

  result = result_position = malloc( strlen( commandline) + 1 );
  result_size = strlen( commandline );

  for (seek_start = commandline;*seek_start;seek_start++)
  {
    if (*seek_start=='$')
    {
      for (seek_end=seek_start+1; *seek_end && *seek_end!='$'; seek_end++);
      if (*seek_end)
      {
        if (seek_end-seek_start < 2 )
          *result_position++='$';
        else
        {
          char *variable_name = malloc( (seek_end-seek_start) );
          strncpy( variable_name, seek_start+1, (seek_end-seek_start)-1 );
          variable_name[(seek_end-seek_start)-1]='\0';

          char *variable_data = settings_get( variable_name );
          if ( variable_data )
          {
            if ( strlen( variable_name )+2 != strlen( variable_data ) )
            {
              unsigned int offset = result_position - result;
              result=realloc( result,
                  ( result_size -
                    ( strlen( variable_name ) + 2 ) ) +
                  strlen( variable_data ) );
              result_position = result + offset;

              result_size += strlen( variable_data) -
                (strlen( variable_name ) + 2);
            }

            strcpy( result_position, variable_data );
            result_position += strlen( variable_data );
          }

          free( variable_name );
        }
        seek_start+=seek_end-seek_start;
      }
    }
    else *result_position++ = *seek_start;
  }
  *result_position='\0';
  return result;
}

char *command_execute(const char *commandline)
{
  char *return_value;
  char *alias;
  CommandFunction *function;
  if (!commandline || !strlen(commandline)) return 0;

  char *newcommandline = command_parse_commandline( commandline );

  char *command = newcommandline;
  while (*command && isblank(*command)) command++;

  char *parameters = command;
  while (*parameters && !isblank(*parameters)) parameters++;
  while (*parameters && isblank(*parameters))
  { *parameters='\0'; parameters++; };

  alias = dictionary_get( _aliases, command );
  if (alias)
  {
    char *aliased_commandline =
      malloc( strlen( alias ) + 1 + strlen( parameters ) + 1 );
    if (strlen(parameters))
      sprintf( aliased_commandline, "%s %s", alias, parameters );
    else
      strcpy( aliased_commandline, alias );
    return_value = command_execute( aliased_commandline );
    free( aliased_commandline );
  }
  else {
    function = dictionary_get( _commands, command );
    if (function)
      return_value = (*function)( parameters );
    else
      return_value = 0;
  }

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
