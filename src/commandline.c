/** Settings implementation.
 * \file settings.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "settings.h"

#include "commandline.h"

typedef struct {
  char *setting_key;
  char *option_short;
  char *option_long;
  char *description_short;
  char *description_long;
  char *value_ifset;
  char *value;
} CommandLine;

Dictionary *_commandline;

static void commandline_done( void );

void commandline_init( void )
{
  _commandline = dictionary_create();

  commandline_add( 0, "h", "help",
      "Display help, or help about a given option",
      "By typing `-h <option>' or `--help <option>' you can get detailed help \
about each option available. When passing an option remember to remove the \
hyphenated prefix. Use `-h help' not `-h --help`.",
      0, 0 );

  atexit( commandline_done );
}

static void commandline_done( void )
{ dictionary_destroy( _commandline ); }

void commandline_add(
    const char *key,
    const char *option_short,
    const char *option_long,
    const char *description_short,
    const char *description_long,
    const char *value_default,
    const char *value_set )
{
  if (!option_short || (key && !value_default) ) return;

  CommandLine *commandline = malloc( sizeof( CommandLine ) );

  if ( key )
  {
    commandline->setting_key = malloc( strlen( key ) + 1 );
    strcpy( commandline->setting_key, key );

    commandline->value = malloc( strlen( value_default ) + 1 );
    strcpy( commandline->value, value_default );

    if ( value_set )
    {
      commandline->value_ifset = malloc( strlen( value_set ) + 1 );
      strcpy( commandline->value_ifset, value_set );
    } else commandline->value_ifset = 0;

  } else commandline->setting_key = 0;

  commandline->option_short = malloc( strlen( option_short ) + 1 );
  strcpy( commandline->option_short, option_short );

  if ( option_long )
  {
    commandline->option_long = malloc( strlen( option_long ) + 1 );
    strcpy( commandline->option_long, option_long );
  } else commandline->option_long = 0;

  if ( description_short )
  {
    commandline->description_short = malloc( strlen( description_short ) + 1 );
    strcpy( commandline->description_short, description_short );
  } else commandline->description_short = 0;

  if ( description_long )
  {
    commandline->description_long = malloc( strlen( description_long ) + 1 );
    strcpy( commandline->description_long, description_long );
  } else commandline->description_long = 0;

  dictionary_set_free( _commandline, option_short, commandline );
  dictionary_set_nofree( _commandline, option_long, commandline );
  
}

/** Display the commandline help.
 * \param app_name The name of the application executable.
 * \param command This is the command to display help for. If this is NULL then
 *    all commands are listed with brief descriptions.
 */
static void commandline_help( char *app_name, const char *command )
{
  if ( command )
  {
    if ( dictionary_haskey( _commandline, command ) )
    {
      CommandLine *commandline = dictionary_get( _commandline, command );

      /* Short description as a header */
      printf( "%s\n",commandline->description_short );

      printf( "\nUsage:\n");
      if ( commandline->value_ifset )
      {
        printf( "\t%s -%s\n", app_name, commandline->option_short );
        if ( commandline->option_long )
          printf( "\t%s --%s\n", app_name, commandline->option_long );
      }
      else
      {
        printf( "\t%s -%s <value>\n", app_name, commandline->option_short );
        if ( commandline->option_long )
          printf( "\t%s --%s <value>\n", app_name, commandline->option_long );

        printf( "\nThe default value, if you do not set this yourself, \
is:\n\t`%s'\n", commandline->value );
      }

      if ( commandline->description_long )
        printf( "\nDetails:\n%s\n", commandline->description_long );

      printf("\n");

    }
    else
    {
      printf("Unknown commandline option `%s'\n\n", command );
      commandline_help( app_name, 0 );
    }
  }
  else
  {
    printf ("Available options:\n");
    unsigned int key_num;
    for ( key_num=0; key_num < dictionary_keyamount( _commandline ); key_num++ )
    {
      char *key = dictionary_key( _commandline, key_num );
      CommandLine *commandline = dictionary_get( _commandline, key );

      /* The commandline dictionary keeps a reference by short and by longname.
       * We only want to display help once for each item. */
      if ( !strcmp( key, commandline->option_long ) ) continue;
      
      printf( commandline->value_ifset ?
          "  -%s, --%s :\n    %s\n" : "  -%s <value>, --%s <value> :\n    %s\n",
        commandline->option_short, commandline->option_long,
        commandline->description_short );
    }
  }
}

unsigned int commandline_process( unsigned int argc, char *argv[] )
{
  unsigned int argument_enum;

  for ( argument_enum = 1; argument_enum < argc; argument_enum++ )
  {
    if ( argv[ argument_enum ][0] == '-' )
    {
      char *option_comparitive;

      if ( argv[ argument_enum ][1] == '-' )
        option_comparitive = &argv[ argument_enum ][2];
      else
        option_comparitive = &argv[ argument_enum ][1];

      if ( dictionary_haskey( _commandline, option_comparitive ) )
      {
        CommandLine *commandline =
          dictionary_get( _commandline, option_comparitive );

        if ( commandline->setting_key )
        {
          if ( commandline->value_ifset )
            settings_set( commandline->setting_key, commandline->value_ifset );
          else
          {
            if ( (argument_enum+1) < argc && strlen( argv[argument_enum+1] ) )
            {
              argument_enum++;

              char *value = malloc( strlen( argv[ argument_enum ] ) );
              strcpy( value, argv[ argument_enum ] );
              settings_set( commandline->setting_key, value );
            }
            else
            {
              printf( "Commandline option `%s' expects a value to be passed \
to it.\n\n", option_comparitive );
              commandline_help( argv[0], option_comparitive );
              return 0;
            }
          }
        }
        else
        {
          /* This condition should only be reached for hard-coded handlers
           * within this commandline handler, like `help' */
          if ( !strcmp( option_comparitive, "h" ) || 
               !strcmp( option_comparitive, "help" ) )
          {
            if ( (argument_enum+1) < argc &&
                 strlen( argv[argument_enum+1] ) &&
                 argv[argument_enum+1][0] != '-' )
              commandline_help( argv[0], argv[ ++argument_enum ] );
            else
              commandline_help( argv[0], 0 );
          }
        }
      }
      else
      {
        printf("Unknown commandline option `%s'\n\n", option_comparitive );
        commandline_help( argv[0], 0 );
        return 0;
      }
    }
  }
  return 1;
}
