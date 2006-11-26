/** Settings implementation.
 * \file settings.c
 */

#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "settings.h"

Dictionary *_settings;

static void settings_done( void );

void settings_init( void )
{
  _settings = dictionary_create();
  /** \todo All default settings should be loaded here */

  settings_set( "version", VERSION );

#ifdef  VWM
  settings_set( "border.colour.fixed.inactive", "grey50" );
#endif

  settings_set( "mouse.focus", "1" );

  settings_set( "text.delay", "1000" );

  settings_set( "text.colour.background", "Wheat" );
  settings_set( "text.colour.foreground", "Black" );

  settings_set( "text.border.colour", "goldenrod" );
  settings_set( "text.border.width", "1" );

  settings_set( "window.move.velocity", "16" );
  settings_set( "window.resize.velocity", "16" );

  atexit( settings_done );
}

static void settings_done( void )
{ dictionary_destroy( _settings ); }

void settings_set( const char *key, const char *value )
{
  char *newvalue;
  if ( value )
  {
    newvalue = malloc( strlen( value ) + 1 );
    strcpy( newvalue, value );
  }
  else
    newvalue = 0;

  dictionary_set_free( _settings, key, newvalue );
}

unsigned int settings_haskey( const char *key )
{ return dictionary_haskey( _settings, key ); }

char *settings_get( const char *key )
{ return dictionary_get( _settings, key ); }

void settings_unset( const char *key )
{ dictionary_unset( _settings, key ); }
